#include <sms.h>
#include <mgb.h>
#include <stdio.h>
// #include <SDL.h>
#include <SDL2/SDL.h>
#include <emscripten.h>
// #include <emscripten/html5.h>

#define INIT_FLAGS (SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER)
#define WINDOW_FLAGS (SDL_WINDOW_ALLOW_HIGHDPI)
#define RENDERER_FLAGS (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
#define AUDIO_FORMAT (AUDIO_S16)
#define SAMPLES (4096)
#define AUDIO_FREQ (48000)
#define AUDIO_ENTRIES (4)

struct AudioData
{
    Sint16 buffer[SAMPLES * 2];
    Uint32 size;
};

static struct SMS_Core sms = {0};
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static SDL_GameController* controller = NULL;
static SDL_PixelFormat* pixel_format = NULL;
static uint32_t pixel_format_enum = 0;
static void* pixel_buffer = NULL;
static int window_w = SMS_SCREEN_WIDTH;
static int window_h = SMS_SCREEN_HEIGHT;
static struct AudioData audio_data[AUDIO_ENTRIES];
static struct SMS_ApuSample sms_audio_samples[SAMPLES];
static bool audio_init = false;
static bool paused = false;
static bool syncfs_running = false;

EMSCRIPTEN_KEEPALIVE
void on_syncfs(void)
{
    syncfs_running = false;
}

static void syncfs(void)
{
    // this is to prevent syncfs from being spammed
    // instead, it is ran every time it has finished running.
    // which is still a lot, but no browser warnings / errors will be
    // flagged this way.
    if (syncfs_running)
    {
        return;
    }

    syncfs_running = true;

    EM_ASM(
        FS.syncfs(false, function (err) {
            if (err) {
                console.log(err);
            }
            _on_syncfs();
        });
    );
}

static void flushsave(void)
{
    if (mgb_save_save_file(NULL))
    {
        syncfs();
    }
}

static void render(void)
{
    SDL_Rect src_rect = {0};
    SDL_Rect dst_rect = {0};
    int display_w = 0;
    int display_h = 0;

    // get the size of the display
    SDL_GetRendererOutputSize(renderer, &display_w, &display_h);
    // get the output size of the sms, fill the src_rect
    SMS_get_pixel_region(&sms, &src_rect.x, &src_rect.y, &src_rect.w, &src_rect.h);
    // center the image (aka, don't stretch to fill screen)
    const int scale = SDL_min(display_w / src_rect.w, display_h / src_rect.h);
    dst_rect.w = src_rect.w * scale;
    dst_rect.h = src_rect.h * scale;
    dst_rect.x = (display_w - dst_rect.w) / 2;
    dst_rect.y = (display_h - dst_rect.h) / 2;

    // finally, render
    SDL_RenderCopy(renderer, texture, &src_rect, &dst_rect);
    SDL_RenderPresent(renderer);
}

static void core_audio_callback(void* user, struct SMS_ApuSample* samples, uint32_t size)
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

        SMS_apu_mixer_s16(samples, adata->buffer, size);
        adata->size = size * 2;
        index = (index + 1) % AUDIO_ENTRIES;
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
    else // sg
    {
        return SDL_MapRGB(pixel_format, r, g, b);
    }
}

static void core_vblank_callback(void* user)
{
    (void)user;

    void* pixels = NULL;
    int pitch = 0;

    SDL_LockTexture(texture, NULL, &pixels, &pitch);
        SDL_ConvertPixels(
            SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT, // w,h
            pixel_format_enum, pixel_buffer, SMS_SCREEN_WIDTH * pixel_format->BytesPerPixel, // src
            pixel_format_enum, pixels, pitch // dst
        );
    SDL_UnlockTexture(texture);
}

static void sdl_audio_callback(void* user, Uint8* data, int len)
{
    (void)user;
    static int index = 0;

    if (len <= 0)
    {
        return;
    }

    if (audio_data[index].size < (Uint32)len / 2)
    {
        memset(data, 0, len);
        return;
    }

    memcpy(data, audio_data[index].buffer, len);
    audio_data[index].size = 0;
    index = (index + 1) % AUDIO_ENTRIES;
}

static void on_key_event(const SDL_KeyboardEvent* e)
{
    const bool down = e->type == SDL_KEYDOWN;
    const bool ctrl = (e->keysym.mod & KMOD_CTRL) > 0;
    const bool shift = (e->keysym.mod & KMOD_SHIFT) > 0;

    if (ctrl)
    {
        if (!down) // only handle keys being released
        {
            return;
        }

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

                case SDL_SCANCODE_O:
                    mgb_load_rom_filedialog();
                    break;

                case SDL_SCANCODE_P:
                    paused ^= 1;
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

        default: break; // silence enum warning
    }
}

static void on_controller_axis_event(const struct SDL_ControllerAxisEvent* e)
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

static void events(void)
{
    SDL_Event e;

    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                on_key_event(&e.key);
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
        }
    }
}

static void run(double delta)
{
    // don't run if a rom isn't loaded
    if (!mgb_has_rom())
    {
        return;
    }

    // don't run if paused
    if (paused)
    {
        return;
    }

    // just in case something sends the main thread to sleep
    // ie, filedialog, then cap the max delta to something reasonable!
    // maybe keep track of deltas here to get an average?
    delta = SDL_min(delta, 1.333333);

    const double cycles = delta * SMS_CYCLES_PER_FRAME;

    SMS_run(&sms, cycles);
}

EMSCRIPTEN_KEEPALIVE
void em_load_rom_data(const char* name, const uint8_t* data, int len)
{
    printf("[EM] loading rom! name: %s len: %d\n", name, len);

    if (len <= 0)
    {
        printf("[EM] invalid rom size!\n");
        return;
    }

    if (mgb_load_rom_data(name, data, len))
    {
        EM_ASM(
            let button = document.getElementById('HackyButton');
            button.style.visibility = "hidden";
        );
        printf("[EM] loaded rom! name: %s len: %d\n", name, len);
    }
}

static void em_loop(void)
{
    static Uint64 start = 0;
    static Uint64 now = 0;

    static const double ms = 1000.0;
    static const double div_60 = ms / 60;
    static double delta = div_60;

    if (start == 0)
    {
        start = SDL_GetPerformanceCounter();
    }

    events();
    run(delta / div_60);
    render();

    flushsave();

    now = SDL_GetPerformanceCounter();
    delta = (double)((now - start)*1000.0) / SDL_GetPerformanceFrequency();
    start = now;
}

static void on_file_callback(const char* file_name, enum CallbackType type, bool result)
{
    bool should_sync = false;

    switch (type)
    {
        case CallbackType_LOAD_ROM:
            break;

        case CallbackType_LOAD_BIOS:
            break;

        case CallbackType_LOAD_SAVE:
            break;

        case CallbackType_LOAD_STATE:
            break;

        case CallbackType_SAVE_SAVE:
            should_sync = result;
            break;

        case CallbackType_SAVE_STATE:
            should_sync = result;
            break;
    }

    if (should_sync)
    {
        syncfs();
    }
}

static void cleanup(void)
{
    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER))
    {
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

int main(void)
{
    if (SDL_Init(INIT_FLAGS) < 0)
    {
        goto fail;
    }

    SDL_DisplayMode display = {0};
    SDL_GetCurrentDisplayMode(0, &display);
    window_w = display.w;
    window_h = display.h;

    window = SDL_CreateWindow("TotalSMS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, WINDOW_FLAGS);
    if (!window)
    {
        goto fail;
    }

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
        .callback = sdl_audio_callback, // todo:
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
        SMS_set_apu_callback(&sms, core_audio_callback, sms_audio_samples, sizeof(sms_audio_samples)/sizeof(sms_audio_samples[0]), AUDIO_FREQ);
    }
    SMS_set_pixels(&sms, pixel_buffer, SMS_SCREEN_WIDTH, pixel_format->BytesPerPixel);

    mgb_init(&sms);
    mgb_set_on_file_callback(on_file_callback);
    mgb_set_save_folder("/save");
    mgb_set_rtc_folder("/rtc");
    mgb_set_state_folder("/state");

    EM_ASM(
        if (!FS.analyzePath("/save").exists) {
            FS.mkdir("/save");
        }
        if (!FS.analyzePath("/rtc").exists) {
            FS.mkdir("/rtc");
        }
        if (!FS.analyzePath("/state").exists) {
            FS.mkdir("/state");
        }

        FS.mount(IDBFS, {}, "/save");
        FS.mount(IDBFS, {}, "/rtc");
        FS.mount(IDBFS, {}, "/state");

        FS.syncfs(true, function (err) {
            if (err) {
                console.log(err);
            }
        });
    );

    // this loop never exits
    emscripten_set_main_loop(em_loop, 0, true);

    cleanup();
    return 0;

fail:
    fprintf(stderr, "SDL FAIL: %s\n", SDL_GetError());
    cleanup();
    return -1;
}
