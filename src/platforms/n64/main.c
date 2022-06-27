#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <libdragon.h>
#include <sms.h>

#define USE_DFS 0

#if USE_DFS == 0
    #include "rom.h"
    #include "romgg.h"
    #include "rom_castle.h"
    #include "rom_flicky.h"
    #include "rom_king_valley.h"
    #include "rom_mean_bean.h"
    #include "rom_fantasy_zone.h"
    #include "rom_golden_axe.h"
    #include "rom_phantasy_star.h"
    #include "rom_sagaia.h"
    // #include "rom_zillion.h"
    // #include "rom_aladdin.h"
    #include "rom_mega_man.h"
    // #include "rom_shining_force.h"
    #include "rom_sonic_chaos.h"
#endif

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

static short* audio_buffer = NULL;
static size_t audio_samples = 0;

static struct SMS_Core sms = {0};
static display_context_t disp = 0;
// See: https://github.com/DragonMinded/libdragon/blob/e8051c77b34b0cafda2bb2e81bb44848b962d5f8/src/display.c#L176
// See: https://github.com/DragonMinded/libdragon/blob/92feeeb9b7d2c03d434a5bee00e82c52159a9a0b/src/rdp.c#L99
extern void *__safe_buffer[3];


static void core_audio_callback(void* user, struct SMS_ApuCallbackData* data)
{
    static size_t size = 0;

    if (!audio_buffer)
    {
        return;
    }

    audio_buffer[size++] = (data->tone0[0] + data->tone1[0] + data->tone2[0] + data->noise[0]) * 128;
    audio_buffer[size++] = (data->tone0[1] + data->tone1[1] + data->tone2[1] + data->noise[1]) * 128;

    if (size == audio_samples)
    {
        #if AUDIO_START
            audio_write_end();
            audio_buffer = audio_write_begin();
        #else
            audio_write(audio_buffer);
        #endif
        size = 0;
    }
}

static void display_message_error(const char* msg)
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

static void display_message_error2(const char* msg, int d)
{
    console_init();
    console_set_render_mode(RENDER_MANUAL);

    for (;;)
    {
        console_clear();
            printf("%s %d", msg, d);
        console_render();
    }
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

static const char* skip_str[] = {"Frameskip: 0", "Frameskip: 1", "Frameskip: 2", "Frameskip: 3", "Frameskip: 4"};

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
        graphics_draw_text(disp, 10, 10, "TotalSMS v0.0.1b");
        graphics_draw_text(disp, 200, 10, skip_str[fps_skip]);
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
    if (cursor < 0) return max-1;
    return (cursor % max);
}

static void display_menu(struct controller_data* kdown, struct controller_data* kheld)
{
    (void)kheld;
    #define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

    struct RomEntry
    {
        const char* name;
        const unsigned char* data;
        size_t size;
    };

    #if USE_DFS == 0
    static const struct RomEntry entries[] =
    {
        { .name = "Sonic The Hedgehog.sms", .data = roms_Sonic_The_Hedgehog__USA__Europe__sms, .size = roms_Sonic_The_Hedgehog__USA__Europe__sms_len },
        { .name = "Fantasy Zone.sms", .data = roms_sms_Fantasy_Zone__World___Rev_2__sms, .size = roms_sms_Fantasy_Zone__World___Rev_2__sms_len },
        { .name = "Golden Axe.sms", .data = roms_sms_Golden_Axe__USA__Europe__sms, .size = roms_sms_Golden_Axe__USA__Europe__sms_len },
        { .name = "Phantasy Star.sms", .data = roms_sms_Phantasy_Star__USA__Europe___Rev_3__sms, .size = roms_sms_Phantasy_Star__USA__Europe___Rev_3__sms_len },
        { .name = "Sagaia.sms", .data = roms_sms_Sagaia__Europe__sms, .size = roms_sms_Sagaia__Europe__sms_len },
        // { .name = "Zillion.sms", .data = roms_sms_Zillion__USA___Rev_1__sms, .size = roms_sms_Zillion__USA___Rev_1__sms_len },
        { .name = "Sonic The Hedgehog - Triple Trouble.gg", .data = roms_gg_Sonic_The_Hedgehog___Triple_Trouble__USA__Europe__gg, .size = roms_gg_Sonic_The_Hedgehog___Triple_Trouble__USA__Europe__gg_len },
        { .name = "Sonic Chaos.gg", .data = roms_gg_Sonic_Chaos__USA__Europe__gg, .size = roms_gg_Sonic_Chaos__USA__Europe__gg_len },
        // { .name = "Aladdin.gg", .data = roms_gg_Aladdin__USA__Europe__gg, .size = roms_gg_Aladdin__USA__Europe__gg_len },
        { .name = "Megaman.gg", .data = roms_gg_Megaman__U______gg, .size = roms_gg_Megaman__U______gg_len },
        // { .name = "Shining Force II.gg", .data = roms_gg_Shining_Force_II___The_Sword_of_Hajya__USA__gg, .size = roms_gg_Shining_Force_II___The_Sword_of_Hajya__USA__gg_len },
        { .name = "Dr Robotnik's MBM.gg", .data = roms_gg_Dr__Robotnik_s_Mean_Bean_Machine__USA__Europe__gg, .size = roms_gg_Dr__Robotnik_s_Mean_Bean_Machine__USA__Europe__gg_len },
        { .name = "The Castle.sg", .data = roms_sg_Castle__The__Japan__sg, .size = roms_sg_Castle__The__Japan__sg_len },
        { .name = "Flicky.sg", .data = roms_sg_Flicky__Japan__sg, .size = roms_sg_Flicky__Japan__sg_len },
        { .name = "King's Valley.sg", .data = roms_sg_King_s_Valley__Taiwan__sg, .size = roms_sg_King_s_Valley__Taiwan__sg_len },
    };
    #endif

    static int cursor = 0;
    static const int max = ARRAY_SIZE(entries);

    graphics_fill_screen(disp, 0);

    if (kdown->c[0].up)
    {
        cursor = menu_update_cursor(cursor-1, max);
    }
    else if (kdown->c[0].down)
    {
        cursor = menu_update_cursor(cursor+1, max);
    }
    else if (kdown->c[0].A)
    {
        // for (int i = 0; i < 3; i++)
        // {
        //     graphics_fill_screen(disp, 0);
        //     display_show(disp);
        //     aquire_and_swap_buffers();
        // }

        if (!SMS_loadrom(&sms, entries[cursor].data, entries[cursor].size, -1))
        {
            display_message_error("failed to load sms rom");
        }

        loadrom_once = true;
        menu = Menu_ROM;
        return;
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

    graphics_draw_text(disp, 10, 10, "TotalSMS v0.0.1b");

    for (int i = 0; i < max; i++)
    {
        if (cursor == i)
        {
            graphics_draw_text(disp, 5, 25 + (i * 15), "->");
            graphics_draw_text(disp, 20, 25 + (i * 15), entries[i].name);
        }
        else
        {
            graphics_draw_text(disp, 5, 25 + (i * 15), entries[i].name);
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
    #if AUDIO_ENABLED
        audio_init(AUDIO_FREQ, AUDIO_BUFFERS);
        audio_pause(0);
        audio_samples = audio_get_buffer_length() * 2; // stereo
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

    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);
    #if AUDIO_ENABLED
        SMS_set_apu_callback(&sms, core_audio_callback, audio_get_frequency());
    #endif
    aquire_and_swap_buffers();

    #if USE_DFS
    if (dfs_init(DFS_DEFAULT_LOCATION) != DFS_ESUCCESS)
    {
        display_message_error("Filesystem failed to start!\n");
    }
    else
    {
        // todo: finish the filebrowser
        dir_t buf;
        dir_findfirst("rom://", &buf);
        display_message_error(buf.d_name);
    }
    #endif

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
    }

    audio_close();
    display_close();
}
