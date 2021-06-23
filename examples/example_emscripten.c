// this is a small-ish example of how you would use my SMS_core
// and how to write a basic "frontend".
#include <sms.h>

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#include <emscripten.h>


enum
{
    WIDTH = SMS_SCREEN_WIDTH,
    HEIGHT = SMS_SCREEN_HEIGHT,

    VOLUME = SDL_MIX_MAXVOLUME / 2,
    SAMPLES = 2048, // theres latency but not too bad for web
    // SAMPLES = 4096, // too much latency, but otherwise perfect
    SDL_AUDIO_FREQ = 96000,
};


static struct SMS_Core sms;
static uint32_t core_pixels[HEIGHT][WIDTH];

static uint8_t rom_data[SMS_ROM_SIZE_MAX] = {0};
static size_t rom_size = 0;
static bool has_rom = false;

static int scale = 2;
static int speed = 1;
static int frameskip_counter = 0;

static const char* rom_path = NULL;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static SDL_AudioDeviceID audio_device = 0;
static SDL_Rect rect = {0};
static SDL_PixelFormat* pixel_format = NULL;
static SDL_GameController* game_controller = NULL;


static void syncfs()
{
    EM_ASM(
        FS.syncfs(function (err) {
            if (err) {
                console.log(err);
            }
        });
    );
}

EMSCRIPTEN_KEEPALIVE
void em_load_rom_data(const char* name, const uint8_t* data, int len)
{
    printf("[EM] loading rom! name: %s len: %d\n", name, len);

    if (len <= 0 || len > sizeof(rom_data))
    {
        return;
    }

    // this is a nice race condition :)
    memcpy(rom_data, data, len);

    rom_size = (size_t)len;

    if (SMS_loadrom(&sms, rom_data, rom_size))
    {
        rom_path = name;
        has_rom = true;
    }
    else
    {
        printf("failed to loadrom\n");
        has_rom = false;
    }
}

static bool get_state_path(char path_out[0x304])
{
    if (!rom_path)
    {
        return false;
    }

    const char* ext = strrchr(rom_path, '.');

    if (ext)
    {
        strncat(path_out, rom_path, ext - rom_path);
    }
    else
    {
        strcat(path_out, rom_path);
    }

    strcat(path_out, ".state");

    return true;
}

static void savestate()
{
    struct SMS_State state;
    char path[0x304] = {"/states/"};

    if (!get_state_path(path))
    {
        return;
    }

    FILE* f = fopen(path, "wb");
    
    if (f)
    {
        SMS_savestate(&sms, &state);
        fwrite(&state, 1, sizeof(state), f);
        fclose(f);

        syncfs();
    }
}

static void loadstate()
{
    struct SMS_State state;
    char path[0x304] = {"/states/"};

    if (!get_state_path(path))
    {
        return;
    }

    FILE* f = fopen(path, "rb");
    
    if (f)
    {
        fread(&state, 1, sizeof(state), f);
        fclose(f);

        SMS_loadstate(&sms, &state);
    }
}

static void run()
{
    if (!has_rom)
    {
        return;
    }

    for (int i = 0; i < speed; ++i)
    {
        SMS_run_frame(&sms);
    }
}

static void filedialog()
{
    EM_ASM(
        let rom_input = document.getElementById("RomFilePicker");
        rom_input.click();
    );
}

static bool is_fullscreen()
{
    const int flags = SDL_GetWindowFlags(window);

    // check if we are already in fullscreen mode
    if (flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP))
    {
        return true;
    }
    else
    {
        return false;
    }
}

static void setup_rect(int w, int h)
{
    if (!w || !h)
    {
        return;
    }
    
    const int scale_w = w / WIDTH;
    const int scale_h = h / HEIGHT;

    // get the min scale
    const int min_scale = scale_w < scale_h ? scale_w : scale_h;

    rect.w = WIDTH * min_scale;
    rect.h = HEIGHT * min_scale;
    rect.x = (w - rect.w);
    rect.y = (h - rect.h);

    // don't divide by zero!
    if (rect.x > 0) rect.x /= 2;
    if (rect.y > 0) rect.y /= 2;
}

static void scale_screen()
{
    SDL_SetWindowSize(window, WIDTH * scale, HEIGHT * scale);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

static void toggle_fullscreen()
{
    // check if we are already in fullscreen mode
    if (is_fullscreen())
    {
        SDL_SetWindowFullscreen(window, 0);
    }
    else
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}

static void on_ctrl_key_event(const SDL_KeyboardEvent* e, bool down)
{
    if (down)
    {
        switch (e->keysym.scancode)
        {
            case SDL_SCANCODE_EQUALS:
            case SDL_SCANCODE_KP_PLUS:
                ++scale;
                scale_screen();
                break;

            case SDL_SCANCODE_MINUS:
            case SDL_SCANCODE_KP_PLUSMINUS:
            case SDL_SCANCODE_KP_MINUS:
                scale = scale > 0 ? scale - 1 : 1;
                scale_screen();
                break;

            case SDL_SCANCODE_1:
            case SDL_SCANCODE_2:
            case SDL_SCANCODE_3:
            case SDL_SCANCODE_4:
            case SDL_SCANCODE_5:
            case SDL_SCANCODE_6:
            case SDL_SCANCODE_7:
            case SDL_SCANCODE_8:
            case SDL_SCANCODE_9:
                speed = (e->keysym.scancode - SDL_SCANCODE_1) + 1;
                break;

            case SDL_SCANCODE_F:
                toggle_fullscreen();
                break;

            case SDL_SCANCODE_L:
                loadstate();
                break;

            case SDL_SCANCODE_S:
                savestate();
                break;

            case SDL_SCANCODE_O:
                filedialog();
                break;

            default: break; // silence enum warning
        }
    }
}

static void on_key_event(const SDL_KeyboardEvent* e)
{
    const bool down = e->type == SDL_KEYDOWN;
    const bool ctrl = (e->keysym.mod & KMOD_CTRL) > 0;

    if (ctrl)
    {
        on_ctrl_key_event(e, down);

        return;
    }

    if (!has_rom)
    {
        return;
    }

    switch (e->keysym.scancode)
    {
        case SDL_SCANCODE_X:        SMS_set_port_a(&sms, JOY1_A_BUTTON, down);      break;
        case SDL_SCANCODE_Z:        SMS_set_port_a(&sms, JOY1_B_BUTTON, down);      break;
        case SDL_SCANCODE_UP:       SMS_set_port_a(&sms, JOY1_UP_BUTTON, down);     break;
        case SDL_SCANCODE_DOWN:     SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, down);   break;
        case SDL_SCANCODE_LEFT:     SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, down);   break;
        case SDL_SCANCODE_RIGHT:    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, down);  break;
        case SDL_SCANCODE_R:        SMS_set_port_b(&sms, RESET_BUTTON, down);       break;
        case SDL_SCANCODE_P:        SMS_set_port_b(&sms, PAUSE_BUTTON, down);       break;

        default: break; // silence enum warning
    }
}

static void on_controller_axis_event(const SDL_ControllerAxisEvent* e)
{
    enum
    {
        deadzone = 8000,
        left     = -deadzone,
        right    = +deadzone,
        up       = -deadzone,
        down     = +deadzone,
    };

    switch (e->axis)
    {
        case SDL_CONTROLLER_AXIS_LEFTX: case SDL_CONTROLLER_AXIS_RIGHTX:
            if (e->value < left)
            {
                SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, true);
                SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, false);
            }
            else if (e->value > right)
            {
                SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, false);
                SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, true);
            }
            else
            {
                SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, false);
                SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, false);
            }
            break;

        case SDL_CONTROLLER_AXIS_LEFTY: case SDL_CONTROLLER_AXIS_RIGHTY:
            if (e->value < up)
            {
                SMS_set_port_a(&sms, JOY1_UP_BUTTON, true);
                SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, false);
            }
            else if (e->value > down)
            {
                SMS_set_port_a(&sms, JOY1_UP_BUTTON, false);
                SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, true);
            }
            else
            {
                SMS_set_port_a(&sms, JOY1_UP_BUTTON, false);
                SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, false);
            }
            break;
    }
}

static void on_controller_event(const SDL_ControllerButtonEvent* e)
{
    const bool down = e->type == SDL_CONTROLLERBUTTONDOWN;

    switch (e->button)
    {
        case SDL_CONTROLLER_BUTTON_A:               SMS_set_port_a(&sms, JOY1_A_BUTTON, down);      break;
        case SDL_CONTROLLER_BUTTON_B:               SMS_set_port_a(&sms, JOY1_B_BUTTON, down);      break;
        case SDL_CONTROLLER_BUTTON_X:               break;
        case SDL_CONTROLLER_BUTTON_Y:               break;
        case SDL_CONTROLLER_BUTTON_START:           SMS_set_port_b(&sms, PAUSE_BUTTON, down);       break;
        case SDL_CONTROLLER_BUTTON_BACK:            SMS_set_port_b(&sms, RESET_BUTTON, down);       break;
        case SDL_CONTROLLER_BUTTON_GUIDE:           break;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:    break;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:       if (down) { filedialog(); } break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      if (down) { filedialog(); } break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:         SMS_set_port_a(&sms, JOY1_UP_BUTTON, down);     break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:       SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, down);   break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:       SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, down);   break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:      SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, down);  break;
    }
}

static void on_controller_device_event(const SDL_ControllerDeviceEvent* e)
{
    switch (e->type)
    {
        case SDL_CONTROLLERDEVICEADDED:
            if (game_controller)
            {
                SDL_GameControllerClose(game_controller);
            }
            game_controller = SDL_GameControllerOpen(e->which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            break;
    }
}

static void on_window_event(const SDL_WindowEvent* e)
{
    switch (e->event)
    {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            setup_rect(e->data1, e->data2);
            break;
    }
}

static void events()
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_QUIT:
                return;
        
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                on_key_event(&e.key);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                on_controller_event(&e.cbutton);
                break;

            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                on_controller_device_event(&e.cdevice);
                break;

            case SDL_CONTROLLERAXISMOTION:
                on_controller_axis_event(&e.caxis);
                break;

            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
                // on_touch_event(&e.tfinger);
                break;
        
            case SDL_WINDOWEVENT:
                on_window_event(&e.window);
                break;
        }
    } 
}

static uint32_t core_on_colour(void* user, uint8_t c)
{
    (void)user;

    const uint8_t r = ((c >> 0) & 0x3) << 6;
    const uint8_t g = ((c >> 2) & 0x3) << 6;
    const uint8_t b = ((c >> 4) & 0x3) << 6;

    return SDL_MapRGB(pixel_format, r, g, b);
}

static void core_on_apu(void* user, struct SMS_ApuCallbackData* data)
{
    (void)user;

    // using buffers because pushing 1 sample at a time seems to
    // cause popping sounds (on my chromebook).
    static int8_t buffer[SAMPLES] = {0};

    static size_t buffer_count = 0;

    // if speedup is enabled, skip x many samples in order to not fill the audio buffer!
    if (speed > 1)
    {
        static int skipped_samples = 0;

        if (skipped_samples < speed - 1)
        {
            ++skipped_samples;
            return;
        }     

        skipped_samples = 0;   
    }

    if (SDL_GetQueuedAudioSize(audio_device) > sizeof(buffer) * 6)
    {
        return;
    }

    buffer[buffer_count++] = data->tone0 + data->tone1 + data->tone2 + data->noise;

    if (buffer_count == sizeof(buffer))
    {
        buffer_count = 0;

        uint8_t samples[sizeof(buffer)] = {0};

        SDL_MixAudioFormat(samples, (const uint8_t*)buffer, AUDIO_S8, sizeof(buffer), VOLUME);

        SDL_QueueAudio(audio_device, samples, sizeof(samples));
    }
}

static void core_on_vblank(void* user)
{
    (void)user;

    ++frameskip_counter;

    if (frameskip_counter >= speed)
    {
        void* pixels; int pitch;

        SDL_LockTexture(texture, NULL, &pixels, &pitch);
        memcpy(pixels, core_pixels, sizeof(core_pixels));
        SDL_UnlockTexture(texture);

        frameskip_counter = 0;
    }
}

static void render()
{
    SDL_RenderClear(renderer);
    if (has_rom)
    {
        SDL_RenderCopy(renderer, texture, NULL, &rect);
    }
    SDL_RenderPresent(renderer);
}

static void cleanup()
{
    if (audio_device)   { SDL_CloseAudioDevice(audio_device); }
    if (pixel_format)   { SDL_free(pixel_format); }
    if (texture)        { SDL_DestroyTexture(texture); }
    if (renderer)       { SDL_DestroyRenderer(renderer); }
    if (window)         { SDL_DestroyWindow(window); }

    SDL_Quit();
}

static void em_loop()
{
    events();
    run();
    render();
}

int main(int argc, char** argv)
{
    EM_ASM(
        FS.mkdir("/saves"); FS.mount(IDBFS, {}, "/saves");
        FS.mkdir("/states"); FS.mount(IDBFS, {}, "/states");

        FS.syncfs(true, function (err) {
            if (err) {
                console.log(err);
            }
        });
    );

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER))
    {
        printf("fail to init sdl\n");
        goto fail;
    }

    if (SDL_GameControllerAddMappingsFromFile("res/gamecontrollerdb.txt"))
    {
        printf("failed to open controllerdb file!\n");
    }

    window = SDL_CreateWindow("TotalSMS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * scale, HEIGHT * scale, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        printf("fail to init window\n");
        goto fail;
    }

    // this doesn't seem to work on chromebook...
    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);

    // save the window pixel format, we will use this to create texure
    // with the native window format so that sdl does not have to do
    // any converting behind the scenes.
    // also, this format will be used for setting the dmg palette as well
    // as the gbc colours.
    const uint32_t pixel_format_enum = SDL_GetWindowPixelFormat(window);

    pixel_format = SDL_AllocFormat(pixel_format_enum);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        printf("fail to init renderer\n");
        goto fail;
    }

    texture = SDL_CreateTexture(renderer, pixel_format_enum, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    if (!texture)
    {
        printf("fail to init texture\n");
        goto fail;
    }

    const SDL_AudioSpec wanted =
    {
        .freq = SDL_AUDIO_FREQ,
        .format = AUDIO_S8,
        .channels = 1,
        .silence = 0,
        .samples = SAMPLES,
        .padding = 0,
        .size = 0,
        .callback = NULL,
        .userdata = NULL,
    };

    SDL_AudioSpec aspec_got = {0};

    audio_device = SDL_OpenAudioDevice(NULL, 0, &wanted, &aspec_got, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

     if (audio_device == 0)
    {
        goto fail;
    }

    printf("[SDL-AUDIO] freq: %d\n", aspec_got.freq);
    printf("[SDL-AUDIO] channels: %d\n", aspec_got.channels);
    printf("[SDL-AUDIO] samples: %d\n", aspec_got.samples);
    printf("[SDL-AUDIO] size: %d\n", aspec_got.size);


    SDL_PauseAudioDevice(audio_device, 0);

    setup_rect(WIDTH * scale, HEIGHT * scale);

    if (!SMS_init(&sms))
    {
        printf("fail to init sms\n");
        goto fail;
    }

    SMS_set_apu_callback(&sms, core_on_apu, NULL, aspec_got.freq + 512);
    SMS_set_vblank_callback(&sms, core_on_vblank, NULL);
    SMS_set_colour_callback(&sms, core_on_colour, NULL);
    SMS_set_pixels(&sms, core_pixels, SMS_SCREEN_WIDTH, pixel_format->BitsPerPixel);

    emscripten_set_main_loop(em_loop, 0, true);

    cleanup();

    return 0;

fail:
    printf("fail %s\n", SDL_GetError());
    cleanup();

    return -1;
}

