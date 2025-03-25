#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <sms.h>
#include <mgb.h>
#include <emscripten.h>

struct Input {
    uint16_t button;
};

struct Gamepad {
    SDL_JoystickID id;
    SDL_Gamepad* pad;
    bool button[SDL_GAMEPAD_BUTTON_COUNT];
    bool last_button[SDL_GAMEPAD_BUTTON_COUNT];
    bool axis[SDL_GAMEPAD_AXIS_COUNT];
    bool last_axis[SDL_GAMEPAD_AXIS_COUNT];
};

typedef struct {
    // sdl stuff
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture_current;
    SDL_Texture* texture_previous;
    SDL_AudioStream* audio_stream;
    const SDL_PixelFormatDetails* pixel_format_details;
    SDL_PixelFormat pixel_format;

    // todo: support multiple controllers.
    struct Gamepad gamepad;

    // vars
    struct SMS_Core sms;
    void* pixel_buffer;
    struct Input inputs[2]; // [0] current [1 previous]

    char rom_path[4096];
    void* rom_data;
    size_t rom_size;

    // config
    int sms_scale;
    int gg_scale;
    int window_w;
    int window_h;
    bool frame_blending;
    bool stretch_screen;

    bool paused;
    bool focus;
    bool quit;
} App;

struct KeyMap {
    SDL_Keycode key;
    enum SMS_Button button;
};

struct HotKeyMap {
    SDL_Keymod mod;
    SDL_Keycode key;
    void(*func)(App* app);
};

struct GamepadButtonMap {
    SDL_GamepadButton key;
    enum SMS_Button button;
};

static void on_file_picker(App* app);
static void on_savestate(App* app);
static void on_loadstate(App* app);
static void on_pause_toggle(App* app);
static void on_fullscreen_toggle(App* app);
static void on_screen_stretch_toggle(App* app);
static void on_frame_blending_toggle(App* app);
static void on_set_pause(App* app, bool enable);
static void on_update_sound_playback_state(App* app);
static bool should_emu_run(const App* app);

static const struct KeyMap KEY_MAP[] = {
    { SDLK_UP, SMS_Button_JOY1_UP },
    { SDLK_LEFT, SMS_Button_JOY1_LEFT },
    { SDLK_DOWN, SMS_Button_JOY1_DOWN },
    { SDLK_RIGHT, SMS_Button_JOY1_RIGHT },
    { SDLK_Z, SMS_Button_JOY1_B },
    { SDLK_X, SMS_Button_JOY1_A },
    { SDLK_RETURN, SMS_Button_PAUSE },

    { SDLK_KP_8, SMS_Button_JOY1_UP },
    { SDLK_KP_4, SMS_Button_JOY1_LEFT },
    { SDLK_KP_2, SMS_Button_JOY1_DOWN },
    { SDLK_KP_6, SMS_Button_JOY1_RIGHT },
    { SDLK_KP_ENTER, SMS_Button_PAUSE },
};

static const struct HotKeyMap HOT_KEY_MAP[] = {
    { SDL_KMOD_CTRL, SDLK_O, on_file_picker }, // open file.
    { SDL_KMOD_CTRL, SDLK_S, on_savestate }, // save state.
    { SDL_KMOD_CTRL, SDLK_L, on_loadstate }, // load state.
    { SDL_KMOD_CTRL, SDLK_P, on_pause_toggle }, // pause.
    { SDL_KMOD_CTRL, SDLK_F, on_fullscreen_toggle }, // fullscreen.
    { SDL_KMOD_SHIFT, SDLK_F, on_screen_stretch_toggle }, // fill the entire screen.
    { SDL_KMOD_SHIFT, SDLK_B, on_frame_blending_toggle }, // blend previous frame.
};

static const struct GamepadButtonMap GAMEPAD_BUTTON_MAP[] = {
    { SDL_GAMEPAD_BUTTON_DPAD_UP, SMS_Button_JOY1_UP },
    { SDL_GAMEPAD_BUTTON_DPAD_LEFT, SMS_Button_JOY1_LEFT },
    { SDL_GAMEPAD_BUTTON_DPAD_DOWN, SMS_Button_JOY1_DOWN },
    { SDL_GAMEPAD_BUTTON_DPAD_RIGHT, SMS_Button_JOY1_RIGHT },
    { SDL_GAMEPAD_BUTTON_SOUTH, SMS_Button_JOY1_B },
    { SDL_GAMEPAD_BUTTON_EAST, SMS_Button_JOY1_A },
    { SDL_GAMEPAD_BUTTON_START, SMS_Button_PAUSE },
};

enum {
    SMS_BPP = 2,
    GG_BPP = 4,
};

static uint32_t sms_converted_palette[1 << SMS_BPP * 3];
static uint32_t gg_converted_palette[1 << GG_BPP * 3];
static uint32_t sg_converted_palette[1 << 4];

static bool syncfs_running = false;

EMSCRIPTEN_KEEPALIVE void on_syncfs(void) {
    syncfs_running = false;
}

EMSCRIPTEN_KEEPALIVE void em_load_rom_data(const char* name, const uint8_t* data, int len) {
    SDL_Log("[EM] loading rom! name: %s len: %d\n", name, len);

    if (len <= 0) {
        SDL_Log("[EM] invalid rom size!\n");
        return;
    }

    if (mgb_load_rom_data(name, data, len)) {
        EM_ASM(
            let button = document.getElementById('HackyButton');
            button.style.visibility = "hidden";
        );
        SDL_Log("[EM] loaded rom! name: %s len: %d\n", name, len);
    }
}

static void syncfs(void) {
    // this is to prevent syncfs from being spammed
    // instead, it is ran every time it has finished running.
    // which is still a lot, but no browser warnings / errors will be
    // flagged this way.
    if (syncfs_running) {
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

static void flushsave(void) {
    if (mgb_save_save_file(NULL)) {
        syncfs();
    }
}

static void input_set(App* app, bool down, uint16_t value) {
    if (down) {
        app->inputs[0].button |= value;
    } else {
        app->inputs[0].button &= ~value;
    }
}

static bool input_is_dirty(const App* app) {
    return app->inputs[0].button != app->inputs[1].button;
}

static void input_apply(App* app) {
    if (!should_emu_run(app)) {
        return;
    }

    SMS_set_buttons(&app->sms, app->inputs[0].button, true);
    SMS_set_buttons(&app->sms, ~app->inputs[0].button, false);
    app->inputs[1] = app->inputs[0];
}

static void mgb_on_file_callback(void* user, const char* file_name, enum CallbackType type, bool result) {
    App* app = user;
    bool should_sync = false;

    switch (type) {
        case CallbackType_LOAD_ROM:
            if (result) {
                on_set_pause(app, false);
            } else {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to load rom", SDL_GetError(), app->window);
            }
            break;

        case CallbackType_LOAD_BIOS:
            if (!result) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to load bios", SDL_GetError(), app->window);
            }
            break;

        case CallbackType_LOAD_SAVE:
            if (!result) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to load save", SDL_GetError(), app->window);
            }
            break;

        case CallbackType_LOAD_STATE:
            if (!result) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to load state", SDL_GetError(), app->window);
            }
            break;

        case CallbackType_SAVE_SAVE:
            if (!result) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to save save file", SDL_GetError(), app->window);
            }
            should_sync = result;
            break;

        case CallbackType_SAVE_STATE:
            if (!result) {
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to save state", SDL_GetError(), app->window);
            }
            should_sync = result;
            break;
    }

    if (should_sync) {
        syncfs();
    }
}

static void* mgb_on_convert_pixels_to_png_format(void* user, int* out_w, int* out_h, int* out_channels)
{
    App* app = user;
    SDL_Rect rect;
    SMS_get_pixel_region(&app->sms, &rect.x, &rect.y, &rect.w, &rect.h);

    const int src_bpp = app->pixel_format_details->bytes_per_pixel;
    const int src_yoff = rect.y * SMS_SCREEN_WIDTH * src_bpp + rect.x * src_bpp;
    const int src_format = app->pixel_format_details->format;
    const int dst_format = SDL_PIXELFORMAT_RGB24;

    *out_w = rect.w;
    *out_h = rect.h;
    *out_channels = 3;
    void* dst = SDL_malloc((*out_w) * (*out_h) * (*out_channels));
    if (!dst)
    {
        return false;
    }

    const bool result = SDL_ConvertPixels(
        rect.w, rect.h,
        src_format, (const uint8_t*)app->pixel_buffer + src_yoff, SMS_SCREEN_WIDTH * src_bpp,
        dst_format, dst, rect.w * *out_channels
    );

    if (!result)
    {
        SDL_Log("failed to convert pixels: %s\n", SDL_GetError());
        free(dst);
        dst = NULL;
    }

    return dst;
}

static void generate_palette(App* app, uint32_t* palette, uint8_t bpp) {
    const unsigned max_rgb = 1 << bpp;
    const unsigned bit_mask = (1 << bpp) - 1;

    for (unsigned r = 0; r < max_rgb; r++) {
        for (unsigned g = 0; g < max_rgb; g++){
            for (unsigned b = 0; b < max_rgb; b++) {
                const unsigned c = (r << bpp * 0) | (g << bpp * 1) | (b << bpp * 2);
                uint8_t rout = r * 255U / bit_mask;
                uint8_t gout = g * 255U / bit_mask;
                uint8_t bout = b * 255U / bit_mask;

                palette[c] = SDL_MapRGB(app->pixel_format_details, NULL, rout ,gout ,bout);
            }
        }
    }
}

static void generate_sg_palette(App* app, uint32_t* palette) {
    struct Colour { uint8_t r,g,b,a; };

    // https://www.smspower.org/uploads/Development/sg1000.txt
    static const struct Colour SG_COLOUR_TABLE[] = {
        {0x00, 0x00, 0x00, 0x00}, // 0: transparent
        {0x00, 0x00, 0x00, 0xFF}, // 1: black
        {0x20, 0xC0, 0x20, 0xFF}, // 2: green
        {0x60, 0xE0, 0x60, 0xFF}, // 3: bright green
        {0x20, 0x20, 0xE0, 0xFF}, // 4: blue
        {0x40, 0x60, 0xE0, 0xFF}, // 5: bright blue
        {0xA0, 0x20, 0x20, 0xFF}, // 6: dark red
        {0x40, 0xC0, 0xE0, 0xFF}, // 7: cyan (?)
        {0xE0, 0x20, 0x20, 0xFF}, // 8: red
        {0xE0, 0x60, 0x60, 0xFF}, // 9: bright red
        {0xC0, 0xC0, 0x20, 0xFF}, // 10: yellow
        {0xC0, 0xC0, 0x80, 0xFF}, // 11: bright yellow
        {0x20, 0x80, 0x20, 0xFF}, // 12: dark green
        {0xC0, 0x40, 0xA0, 0xFF}, // 13: pink
        {0xA0, 0xA0, 0xA0, 0xFF}, // 14: gray
        {0xE0, 0xE0, 0xE0, 0xFF}, // 15: white
    };

    for (int i = 0; i < 16; i++) {
        const struct Colour c = SG_COLOUR_TABLE[i];
        palette[i] = SDL_MapRGBA(app->pixel_format_details, NULL, c.r, c.g, c.b, c.a);
    }
}

static void on_file_picker(App* app) {
    mgb_load_rom_filedialog();
}

static void on_savestate(App* app) {
    mgb_save_state_file(NULL);
}

static void on_loadstate(App* app) {
    mgb_load_state_file(NULL);
}

static void on_pause_toggle(App* app) {
    on_set_pause(app, app->paused ^ 1);
}

static void on_fullscreen_toggle(App* app) {
    const bool is_fullscreen = SDL_WINDOW_FULLSCREEN & SDL_GetWindowFlags(app->window);
    SDL_SetWindowFullscreen(app->window, is_fullscreen ^ 1);
}

static void on_screen_stretch_toggle(App* app) {
    app->stretch_screen ^= 1;
}

static void on_frame_blending_toggle(App* app) {
    app->frame_blending ^= 1;
}

static void on_set_pause(App* app, bool enable) {
    app->paused = enable;
    on_update_sound_playback_state(app);
}

static void on_update_sound_playback_state(App* app) {
    if (should_emu_run(app)) {
        SDL_ResumeAudioStreamDevice(app->audio_stream);
    } else {
        SDL_PauseAudioStreamDevice(app->audio_stream);
    }
}

static bool should_emu_run(const App* app) {
    return mgb_has_rom() && !app->paused && app->focus;
}

static uint32_t core_colour_callback(void* user, uint8_t r, uint8_t g, uint8_t b) {
    App* app = user;
    if (SMS_is_system_type_gg(&app->sms)) {
        return gg_converted_palette[r << 0 | g << 4 | b << 8];
    }
    else {
        return sms_converted_palette[r << 0 | g << 2 | b << 4];
    }
}

static void core_vblank_callback(void* user) {
    App* app = user;
    if (SMS_get_skip_frame(&app->sms)) {
        return;
    }

    void* pixels = NULL; int pitch = 0;
    SDL_LockTexture(app->texture_current, NULL, &pixels, &pitch);
        SDL_Rect rect;
        SMS_get_pixel_region(&app->sms, &rect.x, &rect.y, &rect.w, &rect.h);

        const uint8_t bpp = app->pixel_format_details->bytes_per_pixel;
        const uint32_t src_stride = SMS_SCREEN_WIDTH;
        const uint32_t dst_stride = pitch / bpp;
        const uint32_t pin_off = rect.y * src_stride * bpp + rect.x * bpp;
        const uint32_t pout_off = rect.y * dst_stride * bpp + rect.x * bpp;

        SDL_ConvertPixels(
            rect.w, rect.h, // w,h
            app->pixel_format, (const uint8_t*)app->pixel_buffer + pin_off, SMS_SCREEN_WIDTH * bpp, // src
            app->pixel_format, (uint8_t*)pixels + pout_off, pitch // dst
        );
    SDL_UnlockTexture(app->texture_current);
}

static void core_audio_callback(void* user, int16_t* samples, uint32_t size) {
    App* app = user;
    SDL_PutAudioStreamData(app->audio_stream, samples, size * sizeof(*samples));
}

static void core_input_callback(void* user, int port) {
    App* app = user;
}

static void sdl_audio_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    if (additional_amount) {
        SDL_Log("dropping samples: %d total: %d\n", additional_amount, total_amount);
    }

    SDL_AudioSpec spec;
    if (!SDL_GetAudioStreamFormat(stream, &spec, NULL)) {
        return;
    }

    // 1s worth of audio, 5th of second (83.3ms)
    const int avail = SDL_GetAudioStreamAvailable(stream);
    const int ones = spec.freq * SDL_AUDIO_FRAMESIZE(spec);
    const int buf_size = ones / 5;
    const float freq = spec.freq;

    const float maxDelta = 0.005F;
    const float fillLevel = (float)(buf_size - avail) / (float)buf_size;
    const float dynamicFrequency = ((1.0F - maxDelta) + 2.0F * fillLevel * maxDelta) * freq;
    const float ratio = SDL_clamp(freq / dynamicFrequency, 0.50F, 10.0F);
    SDL_SetAudioStreamFrequencyRatio(stream, ratio);
}

static void sdl_dialog_file_callback(void *userdata, const char * const *filelist, int filter) {
    if (!filelist) {
        SDL_Log("dialog error: %s\n", SDL_GetError());
        return;
    }

    for (unsigned i = 0; filelist[i]; i++) {
        SDL_Log("got: %s\n", filelist[i]);
    }
}

static void sdl_on_key_event(App* app, const SDL_KeyboardEvent* e)
{
    if (e->repeat) {
        return;
    }

    for (size_t i = 0; i < SDL_arraysize(HOT_KEY_MAP); i++) {
        const struct HotKeyMap* p = &HOT_KEY_MAP[i];
        if ((!p->mod || (p->mod & e->mod)) && p->key == e->key && e->down) {
            p->func(app);
            SDL_Log("got hotkey\n");
        }
    }

    // todo: only handle inputs if focused emulator screen.
    //  && !ImGui::IsAnyItemActive()
    if (!(e->mod & (SDL_KMOD_CTRL|SDL_KMOD_SHIFT|SDL_KMOD_ALT|SDL_KMOD_GUI))) {
        for (size_t i = 0; i < SDL_arraysize(KEY_MAP); i++) {
            const struct KeyMap* p = &KEY_MAP[i];
            if (p->key == e->key) {
                input_set(app, e->down, p->button);
            }
        }
    }
}

static void sdl_on_gamepad_axis_event(App* app, const struct SDL_GamepadAxisEvent* e)
{
    // sdl recommends deadzone of 8000
    // auto& controller = app->controllers[e->which];
    struct Gamepad* controller = &app->gamepad;
    controller->axis[e->axis] = SDL_abs(e->value) >= 8000;

    if (controller->last_axis[e->axis] != controller->axis[e->axis]) {
        controller->last_axis[e->axis] = controller->axis[e->axis];
        const bool down = controller->last_axis[e->axis];

        if (e->axis == SDL_GAMEPAD_AXIS_LEFTX) {
            input_set(app, false, SMS_Button_JOY1_LEFT|SMS_Button_JOY1_RIGHT);

            if (e->value < 0) {
                SDL_Log("setting left: %d\n", e->value);
                input_set(app, down, SMS_Button_JOY1_LEFT);
            }
            else if (e->value > 0) {
                SDL_Log("setting right: %d\n", e->value);
                input_set(app, down, SMS_Button_JOY1_RIGHT);
            }
        }
        else if (e->axis == SDL_GAMEPAD_AXIS_LEFTY) {
            input_set(app, false, SMS_Button_JOY1_UP|SMS_Button_JOY1_DOWN);

            if (e->value < 0) {
                SDL_Log("setting up: %d\n", e->value);
                input_set(app, down, SMS_Button_JOY1_UP);
            }
            else if (e->value > 0) {
                SDL_Log("setting down: %d\n", e->value);
                input_set(app, down, SMS_Button_JOY1_DOWN);
            }
        }
    }
}

static void sdl_on_gamepad_device_event(App* app, const SDL_GamepadDeviceEvent* e)
{
    if (e->type == SDL_EVENT_GAMEPAD_ADDED) {
        // struct Gamepad* controller = &app->controller;
        // const auto itr = app->controllers.find(e->which);
        // if (itr == app->controllers.end()) {
        if (!app->gamepad.pad) {
            SDL_Gamepad* controller = SDL_OpenGamepad(e->which);
            if (controller) {
                if (!SDL_SetGamepadSensorEnabled(controller, SDL_SENSOR_ACCEL, true)) {
                    SDL_Log("controller SDL_SENSOR_ACCEL failed: %s\n", SDL_GetError());
                }
                if (!SDL_SetGamepadSensorEnabled(controller, SDL_SENSOR_GYRO, true)) {
                    SDL_Log("controller SDL_SENSOR_GYRO failed: %s\n", SDL_GetError());
                }

                memset(&app->gamepad, 0, sizeof(app->gamepad));
                app->gamepad.pad = controller;
                app->gamepad.id = e->which;
            }
        }
    }
    else if (e->type == SDL_EVENT_GAMEPAD_REMOVED) {
        if (app->gamepad.pad && app->gamepad.id == e->which) {
            SDL_CloseGamepad(app->gamepad.pad);
            memset(&app->gamepad, 0, sizeof(app->gamepad));
        }
    }
}

static void sdl_on_gamepad_button_event(App* app, const struct SDL_GamepadButtonEvent* e)
{
    // auto& controller = app->controllers[e->which];
    struct Gamepad* controller = &app->gamepad;
    controller->button[e->button] = e->down;

    if (controller->last_button[e->button] != controller->button[e->button]) {
        controller->last_button[e->button] = controller->button[e->button];

        for (size_t i = 0; i < SDL_arraysize(GAMEPAD_BUTTON_MAP); i++) {
            const struct GamepadButtonMap* p = &GAMEPAD_BUTTON_MAP[i];
            if (p->key == e->button) {
                input_set(app, e->down, p->button);
            }
        }
    }
}

static void emulator_render(App* app) {
    if (!mgb_has_rom()) {
        return;
    }

    // get the size of the display
    int display_w, display_h;
    SDL_GetRenderOutputSize(app->renderer, &display_w, &display_h);

    // get the output size of the sms
    SDL_Rect rect;
    SMS_get_pixel_region(&app->sms, &rect.x, &rect.y, &rect.w, &rect.h);
    const SDL_FRect src_rect = {.x = rect.x, .y = rect.y, .w = rect.w, .h = rect.h};

    // center the image (aka, don't stretch to fill screen)
    const int scale = SDL_min(display_w / src_rect.w, display_h / src_rect.h);
    SDL_FRect dst_rect;
    dst_rect.w = src_rect.w * scale;
    dst_rect.h = src_rect.h * scale;
    dst_rect.x = (display_w - dst_rect.w) / 2;
    dst_rect.y = (display_h - dst_rect.h) / 2;

    const SDL_FRect* dst_rect_p = app->stretch_screen ? NULL : &dst_rect;

    if (app->frame_blending) {
        SDL_SetTextureBlendMode(app->texture_current, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(app->texture_previous, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(app->texture_previous, 144);

        // render new frame at 100% alpha with the previous frame as 40%
        SDL_RenderTexture(app->renderer, app->texture_current, &src_rect, dst_rect_p);
        SDL_RenderTexture(app->renderer, app->texture_previous, &src_rect, dst_rect_p);

        SDL_Texture* temp = app->texture_current;
        app->texture_current = app->texture_previous;
        app->texture_previous = temp;
    } else {
        SDL_SetTextureBlendMode(app->texture_current, SDL_BLENDMODE_NONE);
        SDL_RenderTexture(app->renderer, app->texture_current, &src_rect, dst_rect_p);
    }
}

static void run(App* app, double delta) {
    // don't run if a rom isn't loaded, paused or lost focus.
    if (!should_emu_run(app)) {
        return;
    }

    // just in case something sends the main thread to sleep
    // ie, filedialog, then cap the max delta to something reasonable!
    // maybe keep track of deltas here to get an average?
    // delta = SDL_min(delta, 1.333333);
    delta = SDL_min(delta, 3.0);
    const double cycles = (double)SMS_CYCLES_PER_FRAME * delta;

    if (input_is_dirty(app)) {
        input_apply(app);
    }
    SMS_run(&app->sms, cycles);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    SDL_Log("Hello World: %s\n", SDL_GetPlatform());

    if (!SDL_SetAppMetadata("TotalSMS", "1.0.0", "com.example.totalsms")) {
        return SDL_APP_FAILURE;
    }

    App* app = SDL_calloc(1, sizeof(*app));
    if (!app) {
        return SDL_APP_FAILURE;
    }
    *appstate = app;

    app->sms_scale = 1;
    app->gg_scale = 1;
    app->window_w = SMS_SCREEN_WIDTH * app->sms_scale;
    app->window_h = SMS_SCREEN_HEIGHT * app->sms_scale;
    app->frame_blending = false;
    app->quit = false;

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        return SDL_APP_FAILURE;
    }
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        return SDL_APP_FAILURE;
    }
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        return SDL_APP_FAILURE;
    }

    SDL_DisplayID display_id = SDL_GetPrimaryDisplay();
    if (!display_id) {
        return SDL_APP_FAILURE;
    }

    const SDL_DisplayMode* display_mode = SDL_GetCurrentDisplayMode(display_id);
    if (!display_mode) {
        return SDL_APP_FAILURE;
    }

    SDL_Log("Display info:\n");
    SDL_Log("\trefresh_rate: %.2f\n", display_mode->refresh_rate);
    SDL_Log("\tpixel_density: %.2f\n", display_mode->pixel_density);
    SDL_Log("\tw: %d\n", display_mode->w);
    SDL_Log("\th: %d\n", display_mode->h);

    // set window to be the entire size of display
    app->window_w = display_mode->w;
    app->window_h = display_mode->h;

    // setup video and textures
    app->window = SDL_CreateWindow("TotalSMS", app->window_w, app->window_h, SDL_WINDOW_HIGH_PIXEL_DENSITY|SDL_WINDOW_RESIZABLE);
    if (!app->window) {
        return SDL_APP_FAILURE;
    }

    app->renderer = SDL_CreateRenderer(app->window, NULL);
    if (!app->renderer) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetRenderVSync(app->renderer, 1)) {
        return SDL_APP_FAILURE;
    }

    app->pixel_format = SDL_GetWindowPixelFormat(app->window);
    app->pixel_format_details = SDL_GetPixelFormatDetails(app->pixel_format);
    if (!app->pixel_format_details) {
        return SDL_APP_FAILURE;
    }

    app->pixel_buffer = SDL_calloc(app->pixel_format_details->bytes_per_pixel, SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT);
    if (!app->pixel_buffer) {
        return SDL_APP_FAILURE;
    }

    app->texture_current = SDL_CreateTexture(app->renderer, app->pixel_format, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    app->texture_previous = SDL_CreateTexture(app->renderer, app->pixel_format, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    if (!app->texture_current || !app->texture_previous) {
        return SDL_APP_FAILURE;
    }

    SDL_SetTextureScaleMode(app->texture_current, SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(app->texture_previous, SDL_SCALEMODE_NEAREST);

    app->audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL, sdl_audio_callback, app);
    if (!app->audio_stream) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_PauseAudioStreamDevice(app->audio_stream)) {
        return SDL_APP_FAILURE;
    }

    SDL_AudioSpec spec;
    if (!SDL_GetAudioStreamFormat(app->audio_stream, NULL, &spec)) {
        return SDL_APP_FAILURE;
    }

    SDL_Log("AUDIO format: %d\n", spec.format);
    SDL_Log("AUDIO channels: %d\n", spec.channels);
    SDL_Log("AUDIO freq: %d\n", spec.freq);

    // allow for any frequency as blip_buf will handle re-sampling.
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    if (!SDL_SetAudioStreamFormat(app->audio_stream, &spec, NULL)) {
        return SDL_APP_FAILURE;
    }

    generate_palette(app, sms_converted_palette, SMS_BPP);
    generate_palette(app, gg_converted_palette, GG_BPP);
    generate_sg_palette(app, sg_converted_palette);

    if (!SMS_init(&app->sms)) {
        return SDL_APP_FAILURE;
    }
    SMS_set_userdata(&app->sms, app);
    SMS_set_colour_callback(&app->sms, core_colour_callback);
    SMS_set_vblank_callback(&app->sms, core_vblank_callback);
    SMS_set_apu_callback(&app->sms, core_audio_callback, spec.freq);
    SMS_set_input_callback(&app->sms, core_input_callback);
    SMS_set_pixels(&app->sms, app->pixel_buffer, SMS_SCREEN_WIDTH, app->pixel_format_details->bytes_per_pixel);
    SMS_set_builtin_palette(&app->sms, sg_converted_palette);

    mgb_init(&app->sms);
    mgb_set_userdata(app);
    mgb_set_on_file_callback(mgb_on_file_callback);
    mgb_set_on_convert_pixels_to_png_format(mgb_on_convert_pixels_to_png_format);
    mgb_set_save_folder("/save");
    mgb_set_state_folder("/state");

    EM_ASM(
        if (!FS.analyzePath("/save").exists) {
            FS.mkdir("/save");
        }
        if (!FS.analyzePath("/state").exists) {
            FS.mkdir("/state");
        }

        FS.mount(IDBFS, {}, "/save");
        FS.mount(IDBFS, {}, "/state");

        FS.syncfs(true, function (err) {
            if (err) {
                console.log(err);
            }
        });
    );

    app->focus = SDL_GetWindowFlags(app->window) & SDL_WINDOW_INPUT_FOCUS;
    on_update_sound_playback_state(app);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    static Uint64 start = 0;
    static Uint64 now = 0;
    static const double TARGET_FRAME_TIME = 1000.0 / 60;
    static double delta = TARGET_FRAME_TIME;

    App* app = appstate;

    if (start == 0) {
        start = SDL_GetPerformanceCounter();
    }

    run(app, delta / TARGET_FRAME_TIME);

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer);

    emulator_render(app);

    if (!SDL_RenderPresent(app->renderer)) {
        return SDL_APP_FAILURE;
    }

    flushsave();

    now = SDL_GetPerformanceCounter();
    delta = (double)((now - start) * 1000ULL) / (double)SDL_GetPerformanceFrequency();
    start = now;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    App* app = appstate;

    switch (event->type) {
        case SDL_EVENT_QUIT:
            app->quit = true;
            break;

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            sdl_on_key_event(app, &event->key);
            break;

        case SDL_EVENT_DROP_FILE:
            // sdl_on_drop_event(app, &event->drop);
            break;

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            sdl_on_gamepad_button_event(app, &event->gbutton);
            break;

        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
            sdl_on_gamepad_device_event(app, &event->gdevice);
            break;

        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            sdl_on_gamepad_axis_event(app, &event->gaxis);
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            app->focus = event->type == SDL_EVENT_WINDOW_FOCUS_GAINED;
            on_update_sound_playback_state(app);
            break;
    }

    if (app->quit) {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    SDL_Log("Exiting...");
    if (result == SDL_APP_FAILURE) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "failed: %s\n", SDL_GetError());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), NULL);
    }

    App* app = appstate;
    if (app) {
        mgb_exit();
        SMS_quit(&app->sms);

        if (app->gamepad.pad) {
            SDL_CloseGamepad(app->gamepad.pad);
        }
        if (app->pixel_buffer) {
            SDL_free(app->pixel_buffer);
        }
        if (app->audio_stream) {
            SDL_DestroyAudioStream(app->audio_stream);
        }
        if (app->texture_current) {
            SDL_DestroyTexture(app->texture_current);
        }
        if (app->texture_previous) {
            SDL_DestroyTexture(app->texture_previous);
        }
        if (app->renderer) {
            SDL_DestroyRenderer(app->renderer);
        }
        if (app->window) {
            SDL_DestroyWindow(app->window);
        }

        SDL_free(app);
    }
}
