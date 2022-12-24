#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
// #include </opt/libdragon/mips64-elf/include/libdragon.h>
// #include <minizip/unzip.h>
#include <unzip.h>
#include <libdragon.h>
#include <sms.h>

// todo: use timer_ticks() for benchmarking
// add timer_ticks() after functions and log the diff
// then sort the logs that way.

#define FS_ENTRIES_MAX 256

static char fs_dir[512];
static dir_t fs_entries[FS_ENTRIES_MAX];
static size_t fs_entries_count;

static uint8_t rom_data[SMS_ROM_SIZE_MAX+1];
static size_t rom_size;

#define FPS_SKIP_MAX (4)
#define AUDIO_FREQ (22050)
#define AUDIO_START (0)
#define AUDIO_BUFFERS (4)
#define SKIP_VSYNC (1)
#define AUDIO_ENABLED (1)

enum Menu
{
    Menu_MAIN,
    Menu_ROM,
};

static enum Menu menu = Menu_MAIN;
static bool loadrom_once = false;
static int fps_skip = 0;
static volatile int fps = 0;
static volatile int previous_fps = 0;

static short* audio_buffer = NULL;
static struct SMS_ApuSample* sms_audio_samples = NULL;
static size_t audio_samples = 0;

static struct SMS_Core sms = {0};
static display_context_t disp = 0;
// See: https://github.com/DragonMinded/libdragon/blob/e8051c77b34b0cafda2bb2e81bb44848b962d5f8/src/display.c#L176
// See: https://github.com/DragonMinded/libdragon/blob/92feeeb9b7d2c03d434a5bee00e82c52159a9a0b/src/rdp.c#L99
extern void *__safe_buffer[3];

enum ExtensionType
{
    ExtensionType_ROM,
    ExtensionType_ZIP,
    ExtensionType_UNK
};

static enum ExtensionType get_extension_type(const char* file_name)
{
    const char* ext = strrchr(file_name, '.');

    if (!ext)
    {
        return ExtensionType_UNK;
    }

    static const struct
    {
        const char* const ext;
        enum ExtensionType type;
    } ext_pairs[] =
    {
        { ".sms", ExtensionType_ROM },
        { ".gg", ExtensionType_ROM },
        { ".sg", ExtensionType_ROM },
        { ".zip", ExtensionType_ZIP },
    };

    #define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

    for (size_t i = 0; i < ARRAY_SIZE(ext_pairs); ++i)
    {
        if (!strcasecmp(ext, ext_pairs[i].ext))
        {
            return ext_pairs[i].type;
        }
    }

    return ExtensionType_UNK;
}

static bool loadzip(const char* path)
{
    unzFile zf = unzOpen64(path);

    if (zf != NULL)
    {
        unz_global_info64 global_info;
        if (UNZ_OK == unzGetGlobalInfo64(zf, &global_info))
        {
            for (uint64_t i = 0; i < global_info.number_entry; i++)
            {
                if (UNZ_OK == unzOpenCurrentFile(zf))
                {
                    unz_file_info64 file_info = {0};
                    char name[256] = {0};

                    if (UNZ_OK == unzGetCurrentFileInfo64(zf, &file_info, name, sizeof(name), NULL, 0, NULL, 0))
                    {
                        if (get_extension_type(name) == ExtensionType_ROM)
                        {
                            if (file_info.uncompressed_size < sizeof(rom_data))
                            {
                                rom_size = file_info.uncompressed_size;
                                unzReadCurrentFile(zf, rom_data, rom_size);
                                unzClose(zf);
                                return true;
                            }
                        }
                    }

                    unzCloseCurrentFile(zf);
                }

                // advance to the next file (if there is one)
                if (i + 1 < global_info.number_entry)
                {
                    unzGoToNextFile(zf); // todo: error handling
                }
            }
        }

        unzClose(zf);
    }

    return false;
}


static bool loadfile(const char* path)
{
    FILE* f = fopen(path, "rb");

    if (f)
    {
        rom_size = fread(rom_data, 1, sizeof(rom_data), f);
        fclose(f);
        return rom_size > 0;
    }

    return false;
}

static void core_audio_callback(void* user, struct SMS_ApuSample* samples, uint32_t size)
{
    if (!audio_buffer)
    {
        return;
    }

    SMS_apu_mixer_s16(samples, audio_buffer, size);

    #if AUDIO_START
        audio_write_end();
        audio_buffer = audio_write_begin();
    #else
        audio_write(audio_buffer);
    #endif
}

_Noreturn static void display_message_error(const char* msg)
{
    console_init();
    console_set_render_mode(RENDER_MANUAL);

    for (;;)
    {
        console_clear();
            printf("%s", msg);
        console_render();
    }
}

static void timer_callback(int ovfl)
{
    // called every 1s
    previous_fps = fps;
    fps = 0;
}

static uint32_t core_colour_callback(void* user, uint8_t r, uint8_t g, uint8_t b)
{
    if (SMS_is_system_type_gg(&sms))
    {
        r = (r << 4) | r;
        g = (g << 4) | g;
        b = (b << 4) | b;

        return graphics_make_color(r, g, b, 0xFF);
    }
    else if (SMS_is_system_type_sms(&sms))
    {
        r = (r << 6) | (r << 4) | (r << 2) | r;
        g = (g << 6) | (g << 4) | (g << 2) | g;
        b = (b << 6) | (b << 4) | (b << 2) | b;

        return graphics_make_color(r, g, b, 0xFF);
    }
    else
    {
        return graphics_make_color(r, g, b, 0xFF);
    }
}

// See: https://github.com/DragonMinded/libdragon/blob/e8051c77b34b0cafda2bb2e81bb44848b962d5f8/src/display.c#L570
// doesn't wait for vblank, likely faster because we will always miss
// vblank window as we dont hit 60fps
extern void display_show_force(display_context_t disp);

static void aquire_and_swap_buffers(void)
{
    while(!(disp = display_lock()));
    graphics_fill_screen(disp, 0);
    SMS_set_pixels(&sms, (short*)__safe_buffer[disp-1]+(320*25)+30, 320, 16);
}

static void display_title(void)
{
    char title[256];
    sprintf(title, "[TotalSMS v0.0.1c] [FPS: %d]\n", previous_fps);
    graphics_draw_text(disp, 10, 10, title);
}

static void core_vblank_callback(void* user)
{
    static int fps_skip_counter = 0;

    if (fps_skip_counter > 0)
    {
        fps_skip_counter--;
        SMS_skip_frame(&sms, true);
    }
    else
    {
        static const char* skip_str[] = {"[skip: 0]", "[skip: 1]", "[skip: 2]", "[skip: 3]", "[skip: 4]"};

        display_title();
        graphics_draw_text(disp, 240, 10, skip_str[fps_skip]);
        graphics_draw_text(disp, 10, 220, "[Z = Menu] [L/R = dec/inc FPS skip]");
        #if SKIP_VSYNC
            display_show_force(disp);
        #else
            display_show(disp);
        #endif
        aquire_and_swap_buffers();
        fps_skip_counter = fps_skip;
        SMS_skip_frame(&sms, false);
    }
}

static int menu_update_cursor(int cursor, int max)
{
    if (cursor < 0)
    {
        return max-1;
    }
    return (cursor % max);
}

// SOURCE: https://github.com/DragonMinded/libdragon/blob/49e6a7d2f2ef88f0be111286f1678ae560fddfa1/examples/dfsdemo/dfsdemo.c#L24
static void chdir( const char * const dirent )
{
    /* Ghetto implementation */
    if( strcmp( dirent, ".." ) == 0 )
    {
        /* Go up one */
        int len = strlen( fs_dir ) - 1;

        /* Stop going past the min */
        if( fs_dir[len] == '/' && fs_dir[len-1] == '/' && fs_dir[len-2] == ':' )
        {
            return;
        }

        if( fs_dir[len] == '/' )
        {
            fs_dir[len] = 0;
            len--;
        }

        while( fs_dir[len] != '/')
        {
            fs_dir[len] = 0;
            len--;
        }
    }
    else
    {
        /* Add to end */
        strcat( fs_dir, dirent );
        strcat( fs_dir, "/" );
    }
}

// SOURCE: https://github.com/DragonMinded/libdragon/blob/49e6a7d2f2ef88f0be111286f1678ae560fddfa1/examples/dfsdemo/dfsdemo.c#L58
static int compare(const void * a, const void * b)
{
    const dir_t *first = (const dir_t *)a;
    const dir_t *second = (const dir_t *)b;

    if(first->d_type == DT_DIR && second->d_type != DT_DIR)
    {
        /* First should be first */
        return -1;
    }

    if(first->d_type != DT_DIR && second->d_type == DT_DIR)
    {
        /* First should be second */
        return 1;
    }

    return strcmp(first->d_name, second->d_name);
}

// SOURCE: https://github.com/DragonMinded/libdragon/blob/49e6a7d2f2ef88f0be111286f1678ae560fddfa1/examples/dfsdemo/dfsdemo.c#L78
static bool scan_dfs(void)
{
    fs_entries_count = 0;

    /* Grab first */
    int ret = dir_findfirst(fs_dir, &fs_entries[fs_entries_count]);

    if( ret != 0 )
    {
        return false;
    }

    fs_entries_count++;

    /* Copy in loop */
    while (fs_entries_count < FS_ENTRIES_MAX)
    {
        /* Grab next */
        ret = dir_findnext(fs_dir,&fs_entries[fs_entries_count]);

        if (ret != 0)
        {
            break;
        }

        // check the extention is what we want
        switch (get_extension_type(fs_entries[fs_entries_count].d_name))
        {
            case ExtensionType_ROM:
            case ExtensionType_ZIP:
                fs_entries_count++;
                break;

            case ExtensionType_UNK:
                printf("skipping file: %s\n", fs_entries[fs_entries_count].d_name);
                break;
        }
    }

    if (fs_entries_count > 1)
    {
        /* Should sort! */
        qsort(fs_entries, fs_entries_count, sizeof(dir_t), compare);
    }

    return true;
}

static void display_menu(struct controller_data* kdown, struct controller_data* kheld)
{
    (void)kheld;

    struct RomEntry
    {
        const char* name;
        const unsigned char* data;
        size_t size;
    };

    static int cursor = 0;
    const int max = fs_entries_count;

    graphics_fill_screen(disp, 0);

    if (kdown->c[0].up)
    {
        cursor = menu_update_cursor(cursor-1, max);
    }
    else if (kdown->c[0].down)
    {
        cursor = menu_update_cursor(cursor+1, max);
    }
    else if (kdown->c[0].A && fs_entries_count > 0)
    {
        if (fs_entries[cursor].d_type == DT_REG)
        {
            char path[512];
            strcpy( path, fs_dir );
            strcat( path, fs_entries[cursor].d_name );

            bool success = false;

            switch (get_extension_type(fs_entries[cursor].d_name))
            {
                case ExtensionType_ROM:
                    success = loadfile(path);
                    break;

                case ExtensionType_ZIP:
                    success = loadzip(path);
                    break;

                case ExtensionType_UNK:
                    break;
            }

            if (success)
            {
                if (!SMS_loadrom(&sms, rom_data, rom_size, -1))
                {
                    char msg[128] = {0};
                    sprintf(msg, "failed to loadrom: %s", fs_entries[cursor].d_name);
                    display_message_error(msg);
                }
                else
                {
                    loadrom_once = true;
                    menu = Menu_ROM;
                }
            }
        }
        else if (fs_entries[cursor].d_type == DT_DIR)
        {
            chdir(fs_entries[cursor].d_name);
            scan_dfs();
            cursor = 0;
        }

        return;
    }
    else if (kdown->c[0].B)
    {
        chdir("..");
        scan_dfs();
        cursor = 0;
    }
    else if (kdown->c[0].Z && loadrom_once)
    {
        for (int i = 0; i < 3; i++)
        {
            graphics_fill_screen(disp, 0);
            display_show(disp);
            aquire_and_swap_buffers();
        }
        menu = Menu_ROM;
        return;
    }

    display_title();

    for (int i = 0; i < max; i++)
    {
        if (cursor == i)
        {
            graphics_draw_text(disp, 5, 25 + (i * 15), "->");
            graphics_draw_text(disp, 20, 25 + (i * 15), fs_entries[i].d_name);
        }
        else
        {
            graphics_draw_text(disp, 5, 25 + (i * 15), fs_entries[i].d_name);
        }
    }

    display_show(disp);
    aquire_and_swap_buffers();
}

static void display_rom(struct controller_data* kdown, struct controller_data* kheld)
{
    if (kdown->c[0].Z)
    {
        graphics_fill_screen(disp, 0);
        menu = Menu_MAIN;
        return;
    }
    else if (kdown->c[0].L)
    {
        fps_skip = fps_skip > 0 ? fps_skip - 1 : 0;
    }
    else if (kdown->c[0].R)
    {
        fps_skip = fps_skip < FPS_SKIP_MAX ? fps_skip + 1 : FPS_SKIP_MAX;
    }

    SMS_set_port_a(&sms, JOY1_UP_BUTTON, kheld->c[0].up);
    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, kheld->c[0].right);
    SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, kheld->c[0].down);
    SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, kheld->c[0].left);
    SMS_set_port_a(&sms, JOY1_A_BUTTON, kheld->c[0].A);
    SMS_set_port_a(&sms, JOY1_B_BUTTON, kheld->c[0].B);
    SMS_set_port_b(&sms, PAUSE_BUTTON, kheld->c[0].start);

    SMS_run(&sms, SMS_CYCLES_PER_FRAME);
}

static void update_joystick_directions(struct controller_data* keys)
{
    #define JOYSTICK_DEAD_ZONE 32

    if( (keys->c[0].x < -JOYSTICK_DEAD_ZONE) )
    {
        keys->c[0].left = true;
    }
    else if ( keys->c[0].x > +JOYSTICK_DEAD_ZONE )
    {
        keys->c[0].right = true;
    }

    if( keys->c[0].y > +JOYSTICK_DEAD_ZONE )
    {
        keys->c[0].up = true;
    }
    else if ( keys->c[0].y < -JOYSTICK_DEAD_ZONE )
    {
        keys->c[0].down = true;
    }
}

int main(void)
{
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
    controller_init();
    timer_init();
    new_timer(TIMER_TICKS(1000000), TF_CONTINUOUS, timer_callback);

    #if AUDIO_ENABLED
        audio_init(AUDIO_FREQ, AUDIO_BUFFERS);
        audio_pause(0);
        audio_samples = audio_get_buffer_length() * 2; // stereo
        sms_audio_samples = malloc(audio_samples * sizeof(struct SMS_ApuSample));
        #if AUDIO_START
            audio_buffer = audio_write_begin();
        #else
            audio_buffer = malloc(audio_samples * sizeof(short));
        #endif
    #endif

    if (!SMS_init(&sms))
    {
        display_message_error("failed to init sms");
    }

    assert(0 && "hello world");

    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);
    #if AUDIO_ENABLED
        SMS_set_apu_callback(&sms, core_audio_callback, sms_audio_samples, audio_samples, audio_get_frequency());
    #endif
    aquire_and_swap_buffers();

    if (dfs_init(DFS_DEFAULT_LOCATION) != DFS_ESUCCESS)
    {
        display_message_error("Filesystem failed to start!\n");
    }

    // first try and mount the romfs
    strcpy(fs_dir, "rom://");
    if (!scan_dfs())
    {
        // if that fails, mount the sd card
        strcpy(fs_dir, "sd://");
        if (!scan_dfs())
        {
            // if that fails, fail early because we have no games :(
            display_message_error("No roms or folders found!\n");
        }
    }

    for (;;)
    {
        controller_scan();
        struct controller_data kheld = get_keys_pressed();
        struct controller_data kdown = get_keys_down();
        update_joystick_directions(&kheld);
        update_joystick_directions(&kdown);

        switch(menu)
        {
            case Menu_MAIN:
                display_menu(&kdown, &kheld);
                break;

            case Menu_ROM:
                display_rom(&kdown, &kheld);
                break;
        }

        fps++;
    }

    // unreachable
    // timer_close();
    // audio_close();
    // display_close();
}
