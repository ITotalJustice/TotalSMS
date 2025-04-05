#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <sms.h>
#include <mgb.h>
#include "args/args.h"

#ifdef EMSCRIPTEN
    #include <emscripten.h>
#endif

enum ArgsId {
    // misc
    ArgsId_help,
    ArgsId_version,

    // file
    ArgsId_rom,
    ArgsId_bios,
    ArgsId_loadstate,

    // video
    ArgsId_fullscreen,
    ArgsId_vsync,
    ArgsId_frame_blending,
    ArgsId_scaler,
    ArgsId_stretch,

    // latency
    ArgsId_runahead,
    ArgsId_runahead_lazy,
};

#define ARGS_ENTRY(_key, _type, _single) \
    { .key = #_key, .id = ArgsId_##_key, .type = _type, .single = _single },

static const struct ArgsMeta ARGS_META[] = {
    ARGS_ENTRY(help, ArgsValueType_NONE, 'h')
    ARGS_ENTRY(version, ArgsValueType_NONE, 'v')

    ARGS_ENTRY(rom, ArgsValueType_STR, 'r')
    ARGS_ENTRY(bios, ArgsValueType_STR, 'b')
    ARGS_ENTRY(loadstate, ArgsValueType_NONE, 0)

    ARGS_ENTRY(fullscreen, ArgsValueType_NONE, 'f')
    ARGS_ENTRY(vsync, ArgsValueType_INT, 0)
    ARGS_ENTRY(frame_blending, ArgsValueType_INT, 0)
    ARGS_ENTRY(scaler, ArgsValueType_INT, 0)
    ARGS_ENTRY(stretch, ArgsValueType_INT, 0)

    ARGS_ENTRY(runahead, ArgsValueType_INT, 0)
    ARGS_ENTRY(runahead_lazy, ArgsValueType_NONE, 0)
};

static const int SDL_VSYNC[] = {
    SDL_RENDERER_VSYNC_DISABLED,
    1, // enable vsync.
    SDL_RENDERER_VSYNC_ADAPTIVE,
};

static const SDL_ScaleMode SDL_SCALER[] = {
    SDL_SCALEMODE_NEAREST,
    SDL_SCALEMODE_LINEAR,
};

static const SDL_RendererLogicalPresentation SDL_STRETCH[] = {
    SDL_LOGICAL_PRESENTATION_DISABLED,
    SDL_LOGICAL_PRESENTATION_STRETCH,
    SDL_LOGICAL_PRESENTATION_LETTERBOX,
    SDL_LOGICAL_PRESENTATION_OVERSCAN,
    SDL_LOGICAL_PRESENTATION_INTEGER_SCALE,
};

static const struct SMS_StateConfig RUNAHEAD_STATE_CONFIG = {
    .fast = true,
    .include_psg_blip = true,
};

struct Runahead {
    uint8_t** states;
    unsigned count;
    unsigned frames;
    size_t state_size;
    bool lock_input; // if true, locks input.
    bool lazy; // if true, uses more optimised version.
};

struct Input {
    uint16_t button;
};

struct Gamepad {
    SDL_JoystickID id;
    SDL_Gamepad* pad;
    bool button[SDL_GAMEPAD_BUTTON_COUNT];
    bool axis[SDL_GAMEPAD_AXIS_COUNT];
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
    bool keys[SDL_SCANCODE_COUNT];

    // todo: support multiple controllers.
    struct Gamepad gamepad;

    // vars
    struct SMS_Core sms;
    void* pixel_buffer;
    struct Runahead runahead;
    struct Input inputs[2]; // [0] current [1 previous]

    // allocated sample buffer for audio callbacks.
    int16_t* sample_data;

    // config
    int sms_scale;
    int gg_scale;
    int window_w;
    int window_h;
    bool frame_blending;

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

static void runahead_init(App* app, unsigned frames);
static void runahead_exit(App* app);
static bool runahead_is_enabled(const App* app);
static void runahead_run_frame(App* app, double delta);

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

#ifdef EMSCRIPTEN
static volatile bool syncfs_running = false;

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
    mgb_save_save_file(NULL);
}
#endif

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

#ifdef EMSCRIPTEN
    if (should_sync) {
        syncfs();
    }
#endif
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
        SDL_free(dst);
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

    for (size_t i = 0; i < SDL_arraysize(SG_COLOUR_TABLE); i++) {
        const struct Colour c = SG_COLOUR_TABLE[i];
        palette[i] = SDL_MapRGBA(app->pixel_format_details, NULL, c.r, c.g, c.b, c.a);
    }
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

static void sdl_poll_emu_key_inputs(App* app) {
    const bool* keys = SDL_GetKeyboardState(NULL);
    const SDL_Keymod mod = SDL_GetModState();

    for (size_t i = 0; i < SDL_arraysize(KEY_MAP); i++) {
        const struct KeyMap* p = &KEY_MAP[i];
        const SDL_Scancode scancode = SDL_GetScancodeFromKey(p->key, NULL);
        const bool down = keys[scancode] && !(mod & (SDL_KMOD_CTRL|SDL_KMOD_SHIFT|SDL_KMOD_ALT|SDL_KMOD_GUI));

        if (app->keys[scancode] != down) {
            app->keys[scancode] = down;
            input_set(app, down, p->button);
        }
    }
}

static void sdl_poll_emu_gamepad_inputs(App* app) {
    struct Gamepad* controller = &app->gamepad;
    for (size_t i = 0; i < SDL_arraysize(GAMEPAD_BUTTON_MAP); i++) {
        const struct GamepadButtonMap* p = &GAMEPAD_BUTTON_MAP[i];
        const bool down = SDL_GetGamepadButton(controller->pad, p->key);

        if (controller->button[p->key] != down) {
            controller->button[p->key] = down;
            input_set(app, down, p->button);
        }
    }
}

static void sdl_poll_emu_axis2_inputs(App* app, SDL_GamepadAxis axis) {
    struct Gamepad* controller = &app->gamepad;
    const int16_t value = SDL_GetGamepadAxis(controller->pad, axis);
    const bool down = SDL_abs(value) >= 8000;

    if (controller->axis[axis] != down) {
        controller->axis[axis] = down;

        if (axis == SDL_GAMEPAD_AXIS_LEFTX) {
            input_set(app, false, SMS_Button_JOY1_LEFT|SMS_Button_JOY1_RIGHT);

            if (value < 0) {
                SDL_Log("setting left: %d\n", value);
                input_set(app, down, SMS_Button_JOY1_LEFT);
            }
            else if (value > 0) {
                SDL_Log("setting right: %d\n", value);
                input_set(app, down, SMS_Button_JOY1_RIGHT);
            }
        }
        else if (axis == SDL_GAMEPAD_AXIS_LEFTY) {
            input_set(app, false, SMS_Button_JOY1_UP|SMS_Button_JOY1_DOWN);

            if (value < 0) {
                SDL_Log("setting up: %d\n", value);
                input_set(app, down, SMS_Button_JOY1_UP);
            }
            else if (value > 0) {
                SDL_Log("setting down: %d\n", value);
                input_set(app, down, SMS_Button_JOY1_DOWN);
            }
        }
    }
}

static void sdl_poll_emu_axis_inputs(App* app) {
    sdl_poll_emu_axis2_inputs(app, SDL_GAMEPAD_AXIS_LEFTX);
    sdl_poll_emu_axis2_inputs(app, SDL_GAMEPAD_AXIS_LEFTY);
}

static void sdl_poll_emu_inputs(App* app) {
    sdl_poll_emu_key_inputs(app);
    sdl_poll_emu_gamepad_inputs(app);
    sdl_poll_emu_axis_inputs(app);
}

static void core_input_callback(void* user, int port) {
    App* app = user;

    // disabled whilst input is locked, used for catching up frames in runahead.
    if (app->runahead.lock_input) {
        return;
    }

    // https://github.com/higan-emu/emulation-articles/tree/master/input/latency
    static uint64_t last_poll_time = 0;
    const uint64_t new_poll_time = SDL_GetTicks();
    const uint64_t poll_max = 5; // 5ms
    if (new_poll_time - last_poll_time < poll_max) {
        return;
    }

    last_poll_time = new_poll_time;
    SDL_PumpEvents();

    sdl_poll_emu_inputs(app);

    if (input_is_dirty(app)) {
        input_apply(app);
    }
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
        mgb_load_rom_file(filelist[i]);
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
}

static void sdl_on_gamepad_axis_event(App* app, const struct SDL_GamepadAxisEvent* e)
{

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
}

// NOTE: this WILL be called on any thread that pushes an event to the queue.
// this is called as soon as an event is pushed, not when events are pumped.
// as such, it is able to handle specific events which cause the main thread
// to no longer be called, as it's being terminated or is the background.
// locks should be used to touch any shared data.
static bool sdl_on_watch_event(void *userdata, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_TERMINATING:
            SDL_Log("[SDL_EVENT_TERMINATING]\n");
            break;
        case SDL_EVENT_LOW_MEMORY:
            SDL_Log("[SDL_EVENT_LOW_MEMORY]\n");
            break;
        case SDL_EVENT_WILL_ENTER_BACKGROUND:
            SDL_Log("[SDL_EVENT_WILL_ENTER_BACKGROUND]\n");
            break;
        case SDL_EVENT_DID_ENTER_BACKGROUND:
            SDL_Log("[SDL_EVENT_DID_ENTER_BACKGROUND]\n");
            break;
        case SDL_EVENT_WILL_ENTER_FOREGROUND:
            SDL_Log("[SDL_EVENT_WILL_ENTER_FOREGROUND]\n");
            break;
        case SDL_EVENT_DID_ENTER_FOREGROUND:
            SDL_Log("[SDL_EVENT_DID_ENTER_FOREGROUND]\n");
            break;
    }

    return true;
}

static void on_file_picker(App* app) {
#ifdef EMSCRIPTEN
    EM_ASM(
        let rom_input = document.getElementById("RomFilePicker");
        rom_input.click();
    );
#else
    static const SDL_DialogFileFilter dialog_filters[] = {{
        .name = "Roms",
        .pattern = "sms;gg;sg;zip",
    }, {
        .name = "Master System",
        .pattern = "sms;zip",
    }, {
        .name = "Game Gear",
        .pattern = "gg;zip",
    }, {
        .name = "SG1000",
        .pattern = "sg;zip",
    }};

    SDL_ShowOpenFileDialog(sdl_dialog_file_callback, app, app->window, dialog_filters, SDL_arraysize(dialog_filters), NULL, false);
#endif
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
    int w, h;
    SDL_RendererLogicalPresentation mode;
    SDL_GetRenderLogicalPresentation(app->renderer, &w, &h, &mode);

    if (mode == SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) {
        mode = SDL_LOGICAL_PRESENTATION_STRETCH;
    } else {
        mode = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
    }

    SDL_SetRenderLogicalPresentation(app->renderer, w, h, mode);
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

    if (app->frame_blending) {
        SDL_SetTextureBlendMode(app->texture_current, SDL_BLENDMODE_NONE);
        SDL_SetTextureBlendMode(app->texture_previous, SDL_BLENDMODE_BLEND);
        SDL_SetTextureAlphaMod(app->texture_previous, 144);

        // render new frame at 100% alpha with the previous frame as 40%
        SDL_RenderTexture(app->renderer, app->texture_current, &src_rect, NULL);
        SDL_RenderTexture(app->renderer, app->texture_previous, &src_rect, NULL);

        SDL_Texture* temp = app->texture_current;
        app->texture_current = app->texture_previous;
        app->texture_previous = temp;
    } else {
        SDL_SetTextureBlendMode(app->texture_current, SDL_BLENDMODE_NONE);
        SDL_RenderTexture(app->renderer, app->texture_current, &src_rect, NULL);
    }
}

static void emulator_run(App* app, size_t cycles, bool skip_audio, bool skip_video, bool lock_input) {
    app->runahead.lock_input = lock_input;
    SMS_skip_audio(&app->sms, skip_audio);
    SMS_skip_frame(&app->sms, skip_video);
    SMS_run(&app->sms, cycles);
}

static void runahead_init(App* app, unsigned frames) {
    if (!frames) {
        runahead_exit(app);
        return;
    }

    app->runahead.frames = frames;
    app->runahead.count = 0;
    app->runahead.states = SDL_malloc(frames * sizeof(*app->runahead.states));
    app->runahead.state_size = SMS_get_state_size(&app->sms, &RUNAHEAD_STATE_CONFIG);
    for (unsigned i = 0; i < app->runahead.frames; i++) {
        app->runahead.states[i] = SDL_malloc(app->runahead.state_size);
    }
}

static void runahead_exit(App* app) {
    if (app->runahead.states) {
        for (unsigned i = 0; i < app->runahead.frames; i++) {
            SDL_free(app->runahead.states[i]);
        }

        SDL_free(app->runahead.states);
    }

    SDL_memset(&app->runahead, 0, sizeof(app->runahead));
}

static bool runahead_is_enabled(const App* app) {
    return app->runahead.frames > 0;
}

// clears frame count so that all new frames must be generated.
// this should be called on input change, loadstate and loadrom.
static void runahead_clear_frames(App* app) {
    app->runahead.count = 0;
}

// run the emulate for a single frame.
// will exit early if the emulate is paused or no rom etc.
// if runahead is disabled, then it will run a frame as normal.
// otherwise, it will
static void runahead_run_frame(App* app, double delta) {
    // don't run if a rom isn't loaded, paused or lost focus.
    if (!should_emu_run(app)) {
        return;
    }

    // just in case something sends the main thread to sleep
    // ie, filedialog, then cap the max delta to something reasonable!
    // maybe keep track of deltas here to get an average?
    // delta = SDL_min(delta, 1.333333);
    delta = SDL_min(delta, 3.0);
    const size_t cycles = SDL_floor((double)SMS_CYCLES_PER_FRAME * delta);
    // const size_t cycles = SMS_CYCLES_PER_FRAME;

    if (!runahead_is_enabled(app)) {
        // run frame as normal
        emulator_run(app, cycles, false, false, false);
    } else {
        sdl_poll_emu_inputs(app);

        if (app->runahead.lazy) {
            if (input_is_dirty(app)) {
                // only loadstate if it's valid
                if (app->runahead.count) {
                    SMS_loadstate(&app->sms, app->runahead.states[0], app->runahead.state_size, &RUNAHEAD_STATE_CONFIG);
                }

                input_apply(app);
                runahead_clear_frames(app);
            }

            // emulate ahead, fill up state array
            if (app->runahead.count < app->runahead.frames) {
                while (app->runahead.count < app->runahead.frames) {
                    emulator_run(app, cycles, true, true, true);
                    SMS_savestate(&app->sms, app->runahead.states[app->runahead.count], app->runahead.state_size, &RUNAHEAD_STATE_CONFIG);
                    app->runahead.count++;
                }
            } else {
                // otherwise, move state array down, over-writting oldest state
                for (unsigned i = 0; i < app->runahead.count - 1; i++) {
                    uint8_t* temp = app->runahead.states[i];
                    app->runahead.states[i] = app->runahead.states[i + 1];
                    app->runahead.states[i + 1] = temp;
                }

                // add new state
                SMS_savestate(&app->sms, app->runahead.states[app->runahead.count - 1], app->runahead.state_size, &RUNAHEAD_STATE_CONFIG);
            }

            emulator_run(app, cycles, false, false, true);
        } else {
            emulator_run(app, cycles, true, true, false);
            SMS_savestate(&app->sms, app->runahead.states[0], app->runahead.state_size, &RUNAHEAD_STATE_CONFIG);

            for (unsigned i = 1; i < app->runahead.frames; i++) {
                emulator_run(app, cycles, true, true, true);
            }

            emulator_run(app, cycles, false, false, true);
            SMS_loadstate(&app->sms, app->runahead.states[0], app->runahead.state_size, &RUNAHEAD_STATE_CONFIG);
        }
    }
}

static SDL_AppResult ShowHelp(SDL_AppResult result, const char* argv0) {
    static const char s[] = {
        "usage: exe [option...] [file]\n\n"

        "Misc Options:\n"
        "    -h, --help             Show help\n"
        "    -v, --version          Show version\n\n"

        "File Options:\n"
        "    -r, --rom FILE         Load rom file.\n"
        "    -b, --bios FILE        Load bios file.\n"
        "    --loadstate            Load savestate.\n\n"

        "Video Options:\n"
        "    -f, --fullscreen       Start in fullscreen.\n"
        "    --vsync\n"
        "        0 - none           Vsync is disabled.\n"
        "        1 - vsync          [Default]. Vsync is enabled.\n"
        "        2 - Adaptive       Adapative vsync is enabled, not always supported.\n"
        "    --frame_blending\n"
        "        0 - none           [Default]. No Blending.\n"
        "        1 - blend          Blend frames n and n-1.\n"
        "    --scaler\n"
        "        0 - nearest        [Default]. Sharp pixels.\n"
        "        1 - bilinear       Bilinear interpolation.\n\n"
        "    --stretch\n"
        "        0 - stretch        Stretched to the output resolution.\n"
        "        1 - letterbox      Scales to fit largest dimension, other dimension is letterboxed with black bars.\n\n"
        "        2 - overscan       Scales to fit smallest dimension, other dimension extends outide.\n\n"
        "        3 - integer        [Default]. Scales in integer multiples.\n\n"

        "Latency Options:\n"
        "    --runahead FRAMES      Runahead n frames, 0 to disable.\n"
        "    --runahead_lazy        More efficient runahead implementation.\n"
    };

    SDL_Log("%s", s);
    return result;
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

    bool show_help = false;
    bool show_version = false;

    const char* rom_file = NULL;
    const char* bios_file = NULL;
    SDL_ScaleMode scaler = SDL_SCALEMODE_NEAREST;
    SDL_RendererLogicalPresentation stretch = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
    int vsync = 1;
    int runahead = 0;
    bool frame_blending = false;
    bool fullscreen = false;
    bool loadstate = false;

    int arg_index = 1;
    ArgsData arg_data;
    ArgsResult arg_result;
    while (!(arg_result = args_parse(&arg_index, argc, argv, ARGS_META, SDL_arraysize(ARGS_META), &arg_data))) {
        switch (ARGS_META[arg_data.meta_index].id) {
            case ArgsId_help:
                show_help = true;
                break;
            case ArgsId_version:
                show_version = true;
                break;

            case ArgsId_rom:
                rom_file = arg_data.value.s;
                break;
            case ArgsId_bios:
                bios_file = arg_data.value.s;
                break;
            case ArgsId_loadstate:
                loadstate = true;
                break;

            case ArgsId_fullscreen:
                fullscreen = true;
                break;
            case ArgsId_vsync:
                vsync = SDL_VSYNC[arg_data.value.i];
                break;
            case ArgsId_frame_blending:
                frame_blending = arg_data.value.i;
                break;
            case ArgsId_scaler:
                scaler = SDL_SCALER[arg_data.value.i];
                break;
            case ArgsId_stretch:
                stretch = SDL_STRETCH[arg_data.value.i];
                break;

            case ArgsId_runahead:
                runahead = arg_data.value.i;
                break;
        }
    }

    if (show_version || show_help) {
        return ShowHelp(SDL_APP_SUCCESS, argv[0]);
    }

    // handle error.
    if (arg_result < 0) {
        if (arg_result == ArgsResult_UNKNOWN_KEY) {
            SDL_SetError("unknown arg [%s]", argv[arg_index]);
        }
        else if (arg_result == ArgsResult_BAD_VALUE) {
            SDL_SetError("arg [--%s] had bad value type [%s]", ARGS_META[arg_data.meta_index].key, arg_data.value.s);
        }
        else if (arg_result == ArgsResult_MISSING_VALUE) {
            SDL_SetError("arg [--%s] requires a value", ARGS_META[arg_data.meta_index].key);
        }
        else {
            SDL_SetError("bad args: %d", arg_result);
        }

        return ShowHelp(SDL_APP_FAILURE, argv[0]);
    }
    // handle warning.
    else if (arg_result == ArgsResult_EXTRA_ARGS) {
        if (!rom_file) {
            rom_file = argv[arg_index];
        }
    }
    else if (arg_index < argc) {
        rom_file = argv[arg_index];
    }

#ifdef EMSCRIPTEN
    app->sms_scale = 1;
    app->gg_scale = 1;
#else
    app->sms_scale = 4;
    app->gg_scale = 5;
#endif
    app->window_w = SMS_SCREEN_WIDTH * app->sms_scale;
    app->window_h = SMS_SCREEN_HEIGHT * app->sms_scale;
    app->frame_blending = frame_blending;
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

    // certian events must be handeld in the below callback.
    if (!SDL_AddEventWatch(sdl_on_watch_event, app)) {
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

    if (!SDL_SetRenderVSync(app->renderer, vsync)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetRenderLogicalPresentation(app->renderer, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT, stretch)) {
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

    SDL_SetTextureScaleMode(app->texture_current, scaler);
    SDL_SetTextureScaleMode(app->texture_previous, scaler);

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

    const size_t sample_data_size = spec.freq / 10 * 2;
    app->sample_data = SDL_malloc(sample_data_size * sizeof(*app->sample_data));
    if (!app->sample_data) {
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
    SMS_set_apu_callback(&app->sms, core_audio_callback, app->sample_data, sample_data_size, spec.freq);
    SMS_set_input_callback(&app->sms, core_input_callback);
    SMS_set_pixels(&app->sms, app->pixel_buffer, SMS_SCREEN_WIDTH, app->pixel_format_details->bytes_per_pixel);
    SMS_set_builtin_palette(&app->sms, sg_converted_palette);

    mgb_init(&app->sms);
    mgb_set_userdata(app);
    mgb_set_on_file_callback(mgb_on_file_callback);
    mgb_set_on_convert_pixels_to_png_format(mgb_on_convert_pixels_to_png_format);

#ifdef EMSCRIPTEN
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
#endif // EMSCRIPTEN

    if (bios_file && !mgb_load_bios_file(bios_file)) {
        SDL_Log("failed to load bios\n");
        return SDL_APP_FAILURE;
    }

    if (rom_file && !mgb_load_rom_file(rom_file)) {
        return SDL_APP_FAILURE;
    }

    if (loadstate && !mgb_load_state_file(NULL)) {
        return SDL_APP_FAILURE;
    }

    if (fullscreen && mgb_has_rom()) {
        on_fullscreen_toggle(app);
    }

    runahead_init(app, runahead);

    app->focus = SDL_GetWindowFlags(app->window) & SDL_WINDOW_INPUT_FOCUS;
    on_update_sound_playback_state(app);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    static Uint64 start = 0;
    static Uint64 now = 0;
    // const double TARGET_FRAME_TIME = 1.0 / 60;
    // pal
    // const double TARGET_FRAME_TIME = 1.0 / 49.701459;
    // ntsc
    const double TARGET_FRAME_TIME = 1.0 / 59.922743;
    double delta = TARGET_FRAME_TIME;

    App* app = appstate;

    if (start == 0) {
        start = SDL_GetPerformanceCounter();
    }

    now = SDL_GetPerformanceCounter();
    delta = (double)(now - start) / (double)SDL_GetPerformanceFrequency();
    start = now;

    runahead_run_frame(app, delta / TARGET_FRAME_TIME);

    if (!SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_RenderClear(app->renderer)) {
        return SDL_APP_FAILURE;
    }

    emulator_render(app);

    if (!SDL_RenderPresent(app->renderer)) {
        return SDL_APP_FAILURE;
    }

    int vsync;
    if (!SDL_GetRenderVSync(app->renderer, &vsync)) {
        return SDL_APP_FAILURE;
    }

#ifdef EMSCRIPTEN
    flushsave();
#endif

    // the below are for testing / simulating different fps targets.
    if (!vsync) {
        // SDL_DelayPrecise(1000000000ULL / 144ULL);
        SDL_DelayPrecise(1000000000ULL / 59.922743);
    }

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
        runahead_exit(app);
        mgb_exit();
        SMS_quit(&app->sms);

        if (app->sample_data) {
            SDL_free(app->sample_data);
        }
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
