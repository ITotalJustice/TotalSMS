// this is a small-ish example of how you would use my SMS_core
// and how to write a basic "frontend".
#include <sms.h>
#include <mgb.h>
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#if defined(PSP) || defined(__SWITCH__) || defined(__WIIU__)
    #define BUILT_IN_ROM 1
#else
    #define BUILT_IN_ROM 0
#endif

#if BUILT_IN_ROM
    #include "rom.h"
    #define ROM roms_Sonic_The_Hedgehog__USA__Europe__sms
    #define ROM_SIZE roms_Sonic_The_Hedgehog__USA__Europe__sms_len
    #define ROM_NAME "rom.sms"
#endif

#define AUDIO_FREQ (48000)
#if defined(PSP)
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
#elif defined(__SWITCH__)
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
#elif defined(__WIIU__)
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
#else
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
#endif
#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
#define SAMPLES 4096

struct AudioData
{
    Sint16 buffer[SAMPLES*2];
    Uint32 size;
};

struct Input {
    uint8_t port[2];
};

static const int sms_scale = 4;
static const int gg_scale = 5;
static struct SMS_Core sms = {0};
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture_current;
static SDL_Texture* texture_previous;
static SDL_AudioStream* audio_stream = NULL;
static SDL_AudioDeviceID audio_device_id = 0;
static SDL_GameController* controller = NULL;
static SDL_PixelFormat* pixel_format = NULL;
static uint32_t pixel_format_enum = 0;
static void* pixel_buffer = NULL;
static int window_w = SMS_SCREEN_WIDTH*sms_scale;
static int window_h = SMS_SCREEN_HEIGHT*sms_scale;
static bool running = true;
static bool audio_init = false;
static bool frame_blending = true;
static struct Input inputs[2]; // [0] current [1 previous]

static void input_set(bool down, uint8_t port, uint8_t value) {
    if (down) {
        inputs[0].port[port] |= value;
    } else {
        inputs[0].port[port] &= ~value;
    }
}

static bool input_is_dirty(void) {
    return inputs[0].port[0] != inputs[1].port[0] || inputs[0].port[1] != inputs[1].port[1];
}

static void input_apply(void) {
    SMS_set_port_a(&sms, inputs[0].port[0], true);
    SMS_set_port_a(&sms, ~inputs[0].port[0], false);
    SMS_set_port_b(&sms, inputs[0].port[1], true);
    SMS_set_port_b(&sms, ~inputs[0].port[1], false);
    inputs[1] = inputs[0];
}

static void render(void)
{
    SDL_Rect src_rect = {0};
    SMS_get_pixel_region(&sms, &src_rect.x, &src_rect.y, &src_rect.w, &src_rect.h);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (frame_blending) {
        SDL_SetTextureBlendMode(texture_current, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(texture_previous, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(texture_previous, 100);

        // render new frame at 100% alpha with the previous frame as 40%
        SDL_RenderCopy(renderer, texture_current, &src_rect, NULL);
        SDL_RenderCopy(renderer, texture_previous, &src_rect, NULL);

        SDL_Texture* temp = texture_current;
        texture_current = texture_previous;
        texture_previous = temp;
    } else {
        SDL_RenderCopy(renderer, texture_current, &src_rect, NULL);
    }

    SDL_RenderPresent(renderer);
}

static void core_audio_callback(void* user, int16_t* samples, uint32_t size)
{
    (void)user;

    SDL_LockAudio();
        SDL_AudioStreamPut(audio_stream, samples, size * sizeof(*samples));
    SDL_UnlockAudio();
}

static uint32_t core_colour_callback(void* user, uint8_t r, uint8_t g, uint8_t b)
{
    (void)user;

    if (SMS_is_system_type_gg(&sms))
    {
        const uint8_t R = r << 4;
        const uint8_t G = g << 4;
        const uint8_t B = b << 4;

        return SDL_MapRGB(pixel_format, R, G, B);
    }
    else
    {
        const uint8_t R = r << 6;
        const uint8_t G = g << 6;
        const uint8_t B = b << 6;

        return SDL_MapRGB(pixel_format, R, G, B);
    }
}

static void core_vblank_callback(void* user)
{
    (void)user;

    if (SMS_get_skip_frame(&sms))
    {
        return;
    }

    void* pixels = NULL; int pitch = 0;
    SDL_LockTexture(texture_current, NULL, &pixels, &pitch);
        SDL_ConvertPixels(
            SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT, // w,h
            pixel_format_enum, pixel_buffer, SMS_SCREEN_WIDTH * pixel_format->BytesPerPixel, // src
            pixel_format_enum, pixels, pitch // dst
        );
    SDL_UnlockTexture(texture_current);

    render();
}

static void sdl_audio_callback(void* user, Uint8* data, int len)
{
    (void)user;

    memset(data, 0, len);
    SDL_AudioStreamGet(audio_stream, data, len);
}

// sdl events
static void on_quit_event(const SDL_QuitEvent* e)
{
    (void)e;
    running = false;
}

static void on_mouse_button_event(const SDL_MouseButtonEvent* e)
{
    (void)e;
}

static void on_mouse_motion_event(const SDL_MouseMotionEvent* e)
{
    (void)e;
}

static void on_key_event(const SDL_KeyboardEvent* e)
{
    const bool down = e->type == SDL_KEYDOWN;
    const bool ctrl = (e->keysym.mod & KMOD_CTRL) > 0;
    const bool shift = (e->keysym.mod & KMOD_SHIFT) > 0;

    if (ctrl)
    {
        if (shift)
        {
        }
        else
        {
            switch (e->keysym.scancode)
            {
                case SDL_SCANCODE_S:
                    mgb_save_state_file(NULL);
                    break;

                case SDL_SCANCODE_L:
                    mgb_load_state_file(NULL);
                    break;

                default: break; // silence enum warning
            }
        }

        return;
    }

    switch (e->keysym.scancode)
    {
        case SDL_SCANCODE_X:        input_set(down, 0, JOY1_B_BUTTON); break;
        case SDL_SCANCODE_Z:        input_set(down, 0, JOY1_A_BUTTON); break;
        case SDL_SCANCODE_UP:       input_set(down, 0, JOY1_UP_BUTTON); break;
        case SDL_SCANCODE_DOWN:     input_set(down, 0, JOY1_DOWN_BUTTON); break;
        case SDL_SCANCODE_LEFT:     input_set(down, 0, JOY1_LEFT_BUTTON); break;
        case SDL_SCANCODE_RIGHT:    input_set(down, 0, JOY1_RIGHT_BUTTON); break;
        case SDL_SCANCODE_R:        input_set(down, 0, RESET_BUTTON); break;
        case SDL_SCANCODE_RETURN:   input_set(down, 0, PAUSE_BUTTON); break;

    #ifndef EMSCRIPTEN
        case SDL_SCANCODE_ESCAPE:
            running = false;
            break;
    #endif // EMSCRIPTEN

        default: break; // silence enum warning
    }
}


static void on_generic_axis_event(const int value, const int axis)
{
    enum
    {
        deadzone = 8000,
        left     = -deadzone,
        right    = +deadzone,
        up       = -deadzone,
        down     = +deadzone,
    };

    switch (axis)
    {
        case SDL_CONTROLLER_AXIS_LEFTX: case SDL_CONTROLLER_AXIS_RIGHTX:
            if (value < left)
            {
                input_set(true, 0, JOY1_LEFT_BUTTON);
                input_set(false, 0, JOY1_RIGHT_BUTTON);
            }
            else if (value > right)
            {
                input_set(false, 0, JOY1_LEFT_BUTTON);
                input_set(true, 0, JOY1_RIGHT_BUTTON);
            }
            else
            {
                input_set(false, 0, JOY1_LEFT_BUTTON);
                input_set(false, 0, JOY1_RIGHT_BUTTON);
            }
            break;

        case SDL_CONTROLLER_AXIS_LEFTY: case SDL_CONTROLLER_AXIS_RIGHTY:
            if (value < up)
            {
                input_set(true, 0, JOY1_UP_BUTTON);
                input_set(false, 0, JOY1_DOWN_BUTTON);
            }
            else if (value > down)
            {
                input_set(false, 0, JOY1_UP_BUTTON);
                input_set(true, 0, JOY1_DOWN_BUTTON);
            }
            else
            {
                input_set(false, 0, JOY1_UP_BUTTON);
                input_set(false, 0, JOY1_DOWN_BUTTON);
            }
            break;
    }
}

static void on_controller_axis_event(const struct SDL_ControllerAxisEvent* e)
{
    on_generic_axis_event(e->value, e->axis);
}

static void on_controller_button_event(const struct SDL_ControllerButtonEvent* e)
{
    const bool down = e->type == SDL_CONTROLLERBUTTONDOWN;

    switch (e->button)
    {
        case SDL_CONTROLLER_BUTTON_A: input_set(down, 0, JOY1_A_BUTTON); break;
        case SDL_CONTROLLER_BUTTON_B: input_set(down, 0, JOY1_B_BUTTON); break;
        case SDL_CONTROLLER_BUTTON_BACK: input_set(down, 1, RESET_BUTTON); break;
        case SDL_CONTROLLER_BUTTON_START: input_set(down, 1, PAUSE_BUTTON); break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: input_set(down, 0, JOY1_UP_BUTTON); break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: input_set(down, 0, JOY1_DOWN_BUTTON); break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: input_set(down, 0, JOY1_LEFT_BUTTON); break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: input_set(down, 0, JOY1_RIGHT_BUTTON); break;
    }
}

static void on_controller_device_event(const struct SDL_ControllerDeviceEvent* e)
{
    switch (e->type)
    {
        case SDL_CONTROLLERDEVICEADDED: {
            SDL_GameController* new_controller = SDL_GameControllerOpen(e->which);
            if (new_controller)
            {
                if (controller)
                {
                    SDL_GameControllerClose(controller);
                }
                controller = new_controller;
                printf("controller opened: %s\n", SDL_GameControllerName(controller));
            }
        } break;

        case SDL_CONTROLLERDEVICEREMOVED:
            if (controller)
            {
                printf("controller closed %s\n", SDL_GameControllerName(controller));
                SDL_GameControllerClose(controller);
                controller = NULL;
            }
            break;

        case SDL_CONTROLLERDEVICEREMAPPED:
            break;
    }
}

static void on_joy_axis_event(const SDL_JoyAxisEvent* e)
{
    on_generic_axis_event(e->value, e->axis);
}

static void on_joy_button_event(const SDL_JoyButtonEvent* e)
{
    (void)e;
}

static void on_joy_hat_motion(const SDL_JoyHatEvent* e)
{
    (void)e;
}

static void on_syswm_event(const SDL_SysWMEvent* e)
{
    (void)e;
}

static void on_user_event(SDL_UserEvent* e)
{
    (void)e;
}

static void events(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_QUIT:
                on_quit_event(&e.quit);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                on_mouse_button_event(&e.button);
                break;

            case SDL_MOUSEMOTION:
                on_mouse_motion_event(&e.motion);
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                on_key_event(&e.key);
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                on_joy_button_event(&e.jbutton);
                break;

            case SDL_JOYAXISMOTION:
                on_joy_axis_event(&e.jaxis);
                break;

            case SDL_JOYHATMOTION:
                on_joy_hat_motion(&e.jhat);
                break;

            case SDL_CONTROLLERAXISMOTION:
                on_controller_axis_event(&e.caxis);
                break;

            case SDL_CONTROLLERBUTTONDOWN:
            case SDL_CONTROLLERBUTTONUP:
                on_controller_button_event(&e.cbutton);
                break;

            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEREMAPPED:
                on_controller_device_event(&e.cdevice);
                break;

            case SDL_SYSWMEVENT:
                on_syswm_event(&e.syswm);
                break;

            case SDL_USEREVENT:
                on_user_event(&e.user);
                break;
        }
    }
}

static void cleanup(void)
{
    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER))
    {
        if (controller)
        {
            SDL_GameControllerClose(controller);
        }

        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    if (SDL_WasInit(SDL_INIT_AUDIO))
    {
        SDL_CloseAudioDevice(audio_device_id);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);

        if (audio_stream)
        {
            SDL_FreeAudioStream(audio_stream);
        }
    }

    if (SDL_WasInit(SDL_INIT_TIMER))
    {
        SDL_QuitSubSystem(SDL_INIT_TIMER);
    }

    if (SDL_WasInit(SDL_INIT_VIDEO))
    {
        if (pixel_format)
        {
            SDL_FreeFormat(pixel_format);
        }
        if (texture_current)
        {
            SDL_DestroyTexture(texture_current);
        }
        if (texture_previous)
        {
            SDL_DestroyTexture(texture_previous);
        }
        if (renderer)
        {
            SDL_DestroyRenderer(renderer);
        }
        if (window)
        {
            SDL_DestroyWindow(window);
        }

        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    mgb_exit();
    SDL_Quit();
}

static void emulate_run(void)
{
    const uint32_t speed = 1;//emulate_get_speed();
    const bool skip_frame = SMS_get_skip_frame(&sms);

    for (uint32_t i = 0; i < speed; i++)
    {
        // skip unseen frames
        if (i + 1 != speed)
        {
            SMS_skip_frame(&sms, true);
        }
        else
        {
            SMS_skip_frame(&sms, skip_frame);
        }

        SMS_run(&sms, SMS_CYCLES_PER_FRAME);
    }
}

// my edit code: RwwYiFuR
struct Runahead {
    struct SMS_State* states;
    uint8_t count;
    uint8_t frames;
};

static struct Runahead g_runahead;

static void runahead_init(uint8_t frames) {
    if (!frames) {
        return;
    }
    g_runahead.frames = frames;
    g_runahead.states = malloc(frames * sizeof(struct SMS_State));
    g_runahead.count = 0;
}

static void runahead_exit(void) {
    if (g_runahead.states) {
        free(g_runahead.states);
    }
    memset(&g_runahead, 0, sizeof(g_runahead));
}

static void run_frame(void) {
    if (!g_runahead.frames) {
        // run frame as normal
        input_apply();
        emulate_run();
    } else {
        if (input_is_dirty()) {
            // only loadstate if it's valid
            if (g_runahead.count) {
                SMS_loadstate(&sms, &g_runahead.states[0], sizeof(g_runahead.states[0]));
            }
            input_apply();
            g_runahead.count = 0;
        }

        // emulate ahead, fill up state array
        if (g_runahead.count < g_runahead.frames) {
            while (g_runahead.count < g_runahead.frames) {
                SMS_skip_audio(&sms, true);
                SMS_skip_frame(&sms, true);
                emulate_run();
                SMS_savestate(&sms, &g_runahead.states[g_runahead.count], sizeof(g_runahead.states[g_runahead.count]), true);
                g_runahead.count++;
            }
        } else {
            // otherwise, move state array down, over-writting oldest state
            for (uint8_t i = 0; i < g_runahead.count - 1; i++) {
                g_runahead.states[i] = g_runahead.states[i + 1];
            }

            // add new state
            SMS_savestate(&sms, &g_runahead.states[g_runahead.count - 1], sizeof(g_runahead.states[g_runahead.count - 1]), true);
        }

        SMS_skip_audio(&sms, false);
        SMS_skip_frame(&sms, false);
        emulate_run();
    }
}

int main(int argc, char** argv)
{
    #if !BUILT_IN_ROM
    if (argc < 2)
    {
        return -1;
    }
    #endif

    // https://github.com/mosra/magnum/issues/184#issuecomment-425952900
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER))
    {
        goto fail;
    }

    // #if defined(__3DS__) || defined(PS2) || defined(DREAMCAST)
    //     joystick = SDL_JoystickOpen(0);
    // #endif

    window = SDL_CreateWindow("TotalSMS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, WINDOW_FLAGS);
    if (!window)
    {
        goto fail;
    }

    SDL_SetWindowMinimumSize(window, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);

    // get native pixel format for window
    pixel_format_enum = SDL_GetWindowPixelFormat(window);
    pixel_format = SDL_AllocFormat(pixel_format_enum);
    if (!pixel_format)
    {
        goto fail;
    }

    pixel_buffer = SDL_calloc(pixel_format->BytesPerPixel, SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT);
    if (!pixel_buffer)
    {
        goto fail;
    }

    renderer = SDL_CreateRenderer(window, -1, RENDERER_FLAGS);
    if (!renderer)
    {
        goto fail;
    }

    texture_current = SDL_CreateTexture(renderer, pixel_format_enum, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    texture_previous = SDL_CreateTexture(renderer, pixel_format_enum, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    if (!texture_current || !texture_previous)
    {
        goto fail;
    }

    SDL_AudioSpec wanted_spec =
    {
        .freq = AUDIO_FREQ,
        .format = AUDIO_S16,
        .channels = 2,
        .samples = SAMPLES,
        .callback = sdl_audio_callback,
        .userdata = NULL,
    };

    SDL_AudioSpec obtained_spec;
    audio_device_id = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &obtained_spec, SDL_AUDIO_ALLOW_ANY_CHANGE);

    if (audio_device_id == 0)
    {
        audio_init = 0;
    }
    else
    {
        audio_stream = SDL_NewAudioStream(
            wanted_spec.format, wanted_spec.channels, obtained_spec.freq,
            obtained_spec.format, obtained_spec.channels, obtained_spec.freq
        );

        audio_init = 1;
        SDL_PauseAudioDevice(audio_device_id, 0);
    }

    SMS_init(&sms);
    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);
    if (audio_init)
    {
        SMS_set_apu_callback(&sms, core_audio_callback, obtained_spec.freq);
    }
    uint32_t palette[16];
    struct Colour { uint8_t r,g,b; };
    // https://www.smspower.org/uploads/Development/sg1000.txt
    const struct Colour SG_COLOUR_TABLE[] =
    {
        {0x00, 0x00, 0x00}, // 0: transparent
        {0x00, 0x00, 0x00}, // 1: black
        {0x20, 0xC0, 0x20}, // 2: green
        {0x60, 0xE0, 0x60}, // 3: bright green
        {0x20, 0x20, 0xE0}, // 4: blue
        {0x40, 0x60, 0xE0}, // 5: bright blue
        {0xA0, 0x20, 0x20}, // 6: dark red
        {0x40, 0xC0, 0xE0}, // 7: cyan (?)
        {0xE0, 0x20, 0x20}, // 8: red
        {0xE0, 0x60, 0x60}, // 9: bright red
        {0xC0, 0xC0, 0x20}, // 10: yellow
        {0xC0, 0xC0, 0x80}, // 11: bright yellow
        {0x20, 0x80, 0x20}, // 12: dark green
        {0xC0, 0x40, 0xA0}, // 13: pink
        {0xA0, 0xA0, 0xA0}, // 14: gray
        {0xE0, 0xE0, 0xE0}, // 15: white
    };
    for (int i = 0; i < 16; i++)
    {
        const struct Colour c = SG_COLOUR_TABLE[i];
        palette[i] = SDL_MapRGB(pixel_format, c.r, c.g, c.b);
    }
    SMS_set_builtin_palette(&sms, palette);
    SMS_set_pixels(&sms, pixel_buffer, SMS_SCREEN_WIDTH, pixel_format->BytesPerPixel);

    mgb_init(&sms);

    if (argc == 3)
    {
        if (!mgb_load_bios_file(argv[2]))
        {
            goto fail;
        }
    }

    #if BUILT_IN_ROM
    if (!mgb_load_rom_data(ROM_NAME, ROM, ROM_SIZE))
    #else
    if (!mgb_load_rom_file(argv[1]))
    #endif
    {
        printf("failed to load rom mgb\n");
        goto fail;
    }

    if (SMS_get_system_type(&sms) == SMS_System_GG)
    {
        SDL_SetWindowMinimumSize(window, GG_SCREEN_WIDTH, GG_SCREEN_HEIGHT);
        SDL_SetWindowSize(window, GG_SCREEN_WIDTH*gg_scale, GG_SCREEN_HEIGHT*gg_scale);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    printf("loaded rom\n");

    runahead_init(0);

    while (running)
    {
        events();
        run_frame();
    }

    runahead_exit();

    cleanup();
    return 0;

fail:
    fprintf(stderr, "SDL FAIL: %s\n", SDL_GetError());
    cleanup();
    return -1;
}
