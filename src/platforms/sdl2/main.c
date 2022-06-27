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
    #define AUDIO_FORMAT AUDIO_S16
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
#elif defined(__SWITCH__)
    #define AUDIO_FORMAT AUDIO_S16
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
#elif defined(__WIIU__)
    #define AUDIO_FORMAT AUDIO_S16
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
#else
    #define AUDIO_FORMAT AUDIO_U8
    #define WINDOW_FLAGS SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
#endif
#define RENDERER_FLAGS SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
#define SAMPLES 4096

struct AudioData
{
#if AUDIO_FORMAT == AUDIO_S16
    Sint16 buffer[SAMPLES*2];
#elif AUDIO_FORMAT == AUDIO_U8
    Uint8 buffer[SAMPLES*2];
#elif AUDIO_FORMAT == AUDIO_S8
    Sint8 buffer[SAMPLES*2];
#else
    #error "unsupported audio format"
#endif
    Uint32 size;
};

#define AUDIO_ENTRIES 4

static const int sms_scale = 4;
static const int gg_scale = 5;
static struct SMS_Core sms = {0};
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static SDL_GameController* controller = NULL;
static SDL_PixelFormat* pixel_format = NULL;
static uint32_t pixel_format_enum = 0;
static void* pixel_buffer = NULL;
static int window_w = SMS_SCREEN_WIDTH*sms_scale;
static int window_h = SMS_SCREEN_HEIGHT*sms_scale;
static struct AudioData audio_data[AUDIO_ENTRIES];
static bool running = true;
static bool audio_init = false;


static void render(void)
{
    SDL_Rect src_rect = {0};
    SMS_get_pixel_region(&sms, &src_rect.x, &src_rect.y, &src_rect.w, &src_rect.h);
    SDL_RenderCopy(renderer, texture, &src_rect, NULL);
    SDL_RenderPresent(renderer);
}

static void core_audio_callback(void* user, struct SMS_ApuCallbackData* data)
{
    (void)user;
    static int index = 0;

    SDL_LockAudio();
        struct AudioData* adata = &audio_data[index];

        if (adata->size >= SAMPLES*2)
        {
            SDL_UnlockAudio();
            return;
        }

    #if AUDIO_FORMAT == AUDIO_S16
        adata->buffer[adata->size++] = (data->tone0[0] + data->tone1[0] + data->tone2[0] + data->noise[0]) * 256;
        adata->buffer[adata->size++] = (data->tone0[1] + data->tone1[1] + data->tone2[1] + data->noise[1]) * 256;
    #elif AUDIO_FORMAT == AUDIO_U8 || AUDIO_FORMAT == AUDIO_S8
        adata->buffer[adata->size++] = (data->tone0[0] + data->tone1[0] + data->tone2[0] + data->noise[0]);
        adata->buffer[adata->size++] = (data->tone0[1] + data->tone1[1] + data->tone2[1] + data->noise[1]);
    #else
        #error "unknown audio format"
    #endif

        if (adata->size >= SAMPLES*2)
        {
            index = (index + 1) % AUDIO_ENTRIES;
        }
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
    else if (SMS_is_system_type_sms(&sms))
    {
        const uint8_t R = r << 6;
        const uint8_t G = g << 6;
        const uint8_t B = b << 6;

        return SDL_MapRGB(pixel_format, R, G, B);
    }
    else
    {
        return SDL_MapRGB(pixel_format, r, g, b);
    }
}

static void core_vblank_callback(void* user)
{
    (void)user;
    void* pixels = NULL; int pitch = 0;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
        SDL_ConvertPixels(
            SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT, // w,h
            pixel_format_enum, pixel_buffer, SMS_SCREEN_WIDTH * pixel_format->BytesPerPixel, // src
            pixel_format_enum, pixels, pitch // dst
        );
    SDL_UnlockTexture(texture);

    render();
}

static void sdl_audio_callback(void* user, Uint8* data, int len)
{
    (void)user;
    static int index = 0;

    if (len <= 0)
    {
        return;
    }

#if AUDIO_FORMAT == AUDIO_S16
    if (audio_data[index].size < (Uint32)len/2)
#else
    if (audio_data[index].size < (Uint32)len)
#endif
    {
        memset(data, 0, len);
        return;
    }

    memcpy(data, audio_data[index].buffer, len);
    audio_data[index].size = 0;
    index = (index + 1) % AUDIO_ENTRIES;
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
        case SDL_SCANCODE_X:        SMS_set_port_a(&sms, JOY1_B_BUTTON, down);      break;
        case SDL_SCANCODE_Z:        SMS_set_port_a(&sms, JOY1_A_BUTTON, down);      break;
        case SDL_SCANCODE_UP:       SMS_set_port_a(&sms, JOY1_UP_BUTTON, down);     break;
        case SDL_SCANCODE_DOWN:     SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, down);   break;
        case SDL_SCANCODE_LEFT:     SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, down);   break;
        case SDL_SCANCODE_RIGHT:    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, down);  break;
        case SDL_SCANCODE_R:        SMS_set_port_b(&sms, RESET_BUTTON, down);       break;
        case SDL_SCANCODE_RETURN:   SMS_set_port_b(&sms, PAUSE_BUTTON, down);       break;

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
                SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, true);
                SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, false);
            }
            else if (value > right)
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
            if (value < up)
            {
                SMS_set_port_a(&sms, JOY1_UP_BUTTON, true);
                SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, false);
            }
            else if (value > down)
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

static void on_controller_axis_event(const struct SDL_ControllerAxisEvent* e)
{
    on_generic_axis_event(e->value, e->axis);
}

static void on_controller_button_event(const struct SDL_ControllerButtonEvent* e)
{
    const bool down = e->type == SDL_CONTROLLERBUTTONDOWN;

    switch (e->button)
    {
        case SDL_CONTROLLER_BUTTON_A: SMS_set_port_a(&sms, JOY1_A_BUTTON, down); break;
        case SDL_CONTROLLER_BUTTON_B: SMS_set_port_a(&sms, JOY1_B_BUTTON, down); break;
        case SDL_CONTROLLER_BUTTON_BACK: SMS_set_port_b(&sms, RESET_BUTTON, down); break;
        case SDL_CONTROLLER_BUTTON_START: SMS_set_port_b(&sms, PAUSE_BUTTON, down); break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: SMS_set_port_a(&sms, JOY1_UP_BUTTON, down); break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, down); break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, down); break;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, down); break;
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
        SDL_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
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
        if (texture)
        {
            SDL_DestroyTexture(texture);
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

int main(int argc, char** argv)
{
    #if !BUILT_IN_ROM
    if (argc < 2)
    {
        return -1;
    }
    #endif

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

    texture = SDL_CreateTexture(renderer, pixel_format_enum, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    if (!texture)
    {
        goto fail;
    }

    SDL_AudioSpec wanted_spec =
    {
        .freq = AUDIO_FREQ,
        .format = AUDIO_FORMAT,
        .channels = 2,
        .samples = SAMPLES,
        .callback = sdl_audio_callback,
        .userdata = NULL,
    };

    if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
    {
        audio_init = 0;
    }
    else
    {
        audio_init = 1;
    }
    SDL_PauseAudio(0);

    SMS_init(&sms);
    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);
    if (audio_init)
    {
        SMS_set_apu_callback(&sms, core_audio_callback, AUDIO_FREQ);
        SMS_set_better_drums(&sms, true);
    }
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

    while (running)
    {
        events();
        SMS_run(&sms, SMS_CYCLES_PER_FRAME);
    }

    cleanup();
    return 0;

fail:
    fprintf(stderr, "SDL FAIL: %s\n", SDL_GetError());
    cleanup();
    return -1;
}
