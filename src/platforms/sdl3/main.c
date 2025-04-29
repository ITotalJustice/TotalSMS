#include "app.h"
#include "text_popup.h"
#include "rewind_bar.h"

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <sms.h>
#include <mgb.h>
#include <lz4.h>
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
    ArgsId_patch,

    // video
    ArgsId_fullscreen,
    ArgsId_vsync,
    ArgsId_frame_blending,
    ArgsId_scaler,
    ArgsId_stretch,
    ArgsId_ratio,
    ArgsId_overscan_fill,

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
    ARGS_ENTRY(patch, ArgsValueType_STR, 0)

    ARGS_ENTRY(fullscreen, ArgsValueType_NONE, 'f')
    ARGS_ENTRY(vsync, ArgsValueType_INT, 0)
    ARGS_ENTRY(frame_blending, ArgsValueType_INT, 0)
    ARGS_ENTRY(scaler, ArgsValueType_INT, 0)
    ARGS_ENTRY(stretch, ArgsValueType_INT, 0)
    ARGS_ENTRY(ratio, ArgsValueType_INT, 0)
    ARGS_ENTRY(overscan_fill, ArgsValueType_BOOL, 0)

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

struct KeyMap {
    SDL_Keycode key;
    enum SMS_Button button;
};

struct RewindKeyMap {
    SDL_Keycode key;
    enum RewindBarButton button;
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

struct RewindGamepadButtonMap {
    SDL_GamepadButton key;
    enum RewindBarButton button;
};

static void on_file_picker(App* app);
static void on_savestate(App* app);
static void on_loadstate(App* app);
static void on_pause_toggle(App* app);
static void on_rewind_toggle(App* app);
static void on_fullscreen_toggle(App* app);
static void on_screen_stretch_toggle(App* app);
static void on_frame_blending_toggle(App* app);
static void on_speed_increase(App* app);
static void on_speed_decrease(App* app);
static void on_speed_reset(App* app);

static void on_set_pause(App* app, bool enable);
static void on_set_rewind(App* app, bool enable);
static void on_set_speed(App* app, int speed);

static void on_update_sound_playback_state(App* app);
static void on_speed_change(App* app);
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

static const struct RewindKeyMap REWIND_KEY_MAP[] = {
    { SDLK_LEFT, RewindBarButton_Left },
    { SDLK_RIGHT, RewindBarButton_Right },
    { SDLK_Z, RewindBarButton_Back },
    { SDLK_X, RewindBarButton_OK },
};

static const struct HotKeyMap HOT_KEY_MAP[] = {
    { SDL_KMOD_CTRL, SDLK_O, on_file_picker }, // open file.
    { SDL_KMOD_CTRL, SDLK_S, on_savestate }, // save state.
    { SDL_KMOD_CTRL, SDLK_L, on_loadstate }, // load state.
    { SDL_KMOD_CTRL, SDLK_P, on_pause_toggle }, // pause.
    { SDL_KMOD_CTRL, SDLK_R, on_rewind_toggle }, // rewind.
    { SDL_KMOD_CTRL, SDLK_F, on_fullscreen_toggle }, // fullscreen.
    { SDL_KMOD_SHIFT, SDLK_F, on_screen_stretch_toggle }, // fill the entire screen.
    { SDL_KMOD_SHIFT, SDLK_B, on_frame_blending_toggle }, // blend previous frame.
    { SDL_KMOD_SHIFT, SDLK_EQUALS, on_speed_increase }, // increase speed by 1.
    { SDL_KMOD_SHIFT, SDLK_MINUS, on_speed_decrease }, // decrease speed by 1.
    { SDL_KMOD_SHIFT, SDLK_SPACE, on_speed_reset }, // reset speed back to 1.
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

static const struct RewindGamepadButtonMap REWIND_GAMEPAD_BUTTON_MAP[] = {
    { SDL_GAMEPAD_BUTTON_DPAD_LEFT, RewindBarButton_Left },
    { SDL_GAMEPAD_BUTTON_DPAD_RIGHT, RewindBarButton_Right },
    { SDL_GAMEPAD_BUTTON_SOUTH, RewindBarButton_Back },
    { SDL_GAMEPAD_BUTTON_EAST, RewindBarButton_OK },
};

enum { SPEED_DEFAULT_INDEX = 3 };

static const float SPEED_TABLE[] = {
    0.25, 0.50, 0.75,
    1.00, // default.
    1.25, 1.50, 2.00, 3.00, 4.00,
};

enum {
    SMS_BPP = 2,
    GG_BPP = 4,
};

static uint32_t sms_converted_palette[1 << SMS_BPP * 3];
static uint32_t gg_converted_palette[1 << GG_BPP * 3];
static uint32_t sg_converted_palette[1 << 4];

#ifdef EMSCRIPTEN
EMSCRIPTEN_KEEPALIVE void em_load_rom_data(const char* name, const uint8_t* data, int len) {
    SDL_Log("[EM] loading rom! name: %s len: %d\n", name, len);

    if (len <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "[EM] invalid rom size!\n");
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

static void flushsave(void) {
    mgb_save_save_file(NULL);
}
#endif

static void input_set(App* app, bool down, uint16_t value) {
    if (!should_emu_run(app)) {
        return;
    }

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

static size_t compressor_size_lz4(size_t src_size) {
    return LZ4_compressBound(src_size);
}

static size_t compressor_lz4(const void* src_data, void* dst_data, size_t src_size, size_t dst_size, bool inflate_mode) {
    int result;

    if (inflate_mode) {
        result = LZ4_decompress_safe(src_data, dst_data, src_size, dst_size);
    } else {
        result = LZ4_compress_default(src_data, dst_data, src_size, dst_size);
    }

    if (result <= 0) {
        return 0;
    }

    return result;
}

bool rewind_push_new_frame(App* app) {
    SDL_memcpy(app->rewind_pixel_buffer, app->pixel_buffer[app->pixel_buffer_index], app->pixel_buffer_size);

    if (!SMS_savestate(&app->sms, app->rewind_state_buffer, app->rewind_state_buffer_size, &app->rewind_state_config)) {
        return false;
    }

    if (!rewind_push(app->rewind, app->rewind_buffer, app->rewind_buffer_size)) {
        return false;
    }

    // enable to see compression ratio.
#if 0
    const size_t compressed_size = rewind_get_size_last(app->rewind);
    SDL_Log("compression %.2f%%\n", ((double)compressed_size / (double)app->rewind_buffer_size) * 100.0);
#endif

    return true;
}

static void on_rom_load(App* app) {
    rewind_bar_set_open(app, false);

    // free rewind and rewind buffer.
    if (app->rewind) {
        rewind_close(app->rewind);
    }

    if (app->rewind_buffer) {
        SDL_free(app->rewind_buffer);
        app->rewind_pixel_buffer = NULL;
        app->rewind_state_buffer = NULL;
    }

    // reset rewind state and create savestate config.
    app->rewind_counter = 0;
    app->rewind_should_push = false;
    app->rewind_state_config.include_psg_blip = true;
    app->rewind_state_config.fast = false;

    // allocate new rewind buffer.
    app->rewind_buffer_size = app->pixel_buffer_size;
    app->rewind_buffer_size += SMS_get_state_size(&app->sms, &app->rewind_state_config);
    app->rewind_buffer = SDL_malloc(app->rewind_buffer_size);

    // setup pointers.
    app->rewind_pixel_buffer = app->rewind_buffer;
    app->rewind_pixel_buffer_size = app->pixel_buffer_size;
    app->rewind_state_buffer = (uint8_t*)app->rewind_buffer + app->rewind_pixel_buffer_size;
    app->rewind_state_buffer_size = app->rewind_buffer_size - app->rewind_pixel_buffer_size;

    // finally, create rewind.
    const size_t count = 60 * app->rewind_num_seconds / app->rewind_keyframe_interval;
    app->rewind = rewind_init(app->rewind_buffer_size, count, compressor_lz4, compressor_size_lz4);

    // we don't want to play left over audio data from the previous game.
    SDL_ClearAudioStream(app->audio_stream);

    // clear the frame buffers.
    SDL_memset(app->pixel_buffer[0], 0, app->pixel_buffer_size);
    SDL_memset(app->pixel_buffer[1], 0, app->pixel_buffer_size);

    // resume emulator when a rom is loaded.
    on_set_pause(app, false);
}

static void mgb_on_file_callback(void* user, const char* file_name, enum CallbackType type, bool result) {
    App* app = user;

    switch (type) {
        case CallbackType_LOAD_ROM:
            if (result) {
                on_rom_load(app);
                text_popup_push(TextPopupType_INFO, "Loaded Rom");
            } else {
                text_popup_push(TextPopupType_ERROR, "Failed to load rom");
            }
            break;

        case CallbackType_LOAD_BIOS:
            if (result) {
                text_popup_push(TextPopupType_INFO, "Loaded Bios");
            } else {
                text_popup_push(TextPopupType_ERROR, "Failed to load bios");
            }
            break;

        case CallbackType_LOAD_SAVE:
            if (result) {
                text_popup_push(TextPopupType_INFO, "Loaded Save");
            } else {
                text_popup_push(TextPopupType_ERROR, "Failed to load save");
            }
            break;

        case CallbackType_LOAD_STATE:
            if (result) {
                on_set_rewind(app, false);
                text_popup_push(TextPopupType_INFO, "Loaded State");
            } else {
                text_popup_push(TextPopupType_ERROR, "Failed to load state");
            }
            break;

        case CallbackType_SAVE_SAVE:
            if (result) {
            } else {
                text_popup_push(TextPopupType_ERROR, "Failed to save save file");
            }
            break;

        case CallbackType_SAVE_STATE:
            if (result) {
                text_popup_push(TextPopupType_INFO, "Saved State");
            } else {
                text_popup_push(TextPopupType_ERROR, "Failed to save state");
            }
            break;

        case CallbackType_PATCH_ROM:
            if (result) {
                text_popup_push(TextPopupType_INFO, "Patched Rom");
            } else {
                text_popup_push(TextPopupType_ERROR, "Failed to patch rom");
            }
            break;
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
        return NULL;
    }

    const bool result = SDL_ConvertPixels(
        rect.w, rect.h,
        src_format, (const uint8_t*)app->pixel_buffer[app->pixel_buffer_index] + src_yoff, SMS_SCREEN_WIDTH * src_bpp,
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

// generates full colour range based on the bit depth.
// ie, for SMS 2 bit depth will produce 0, 85, 170, 255.
static void generate_palette(App* app, uint32_t* palette, uint8_t bpp) {
    const unsigned max_rgb = 1 << bpp;
    const unsigned bit_mask = (1 << bpp) - 1;

    for (unsigned r = 0; r < max_rgb; r++) {
        for (unsigned g = 0; g < max_rgb; g++){
            for (unsigned b = 0; b < max_rgb; b++) {
                const unsigned index = (r << bpp * 0) | (g << bpp * 1) | (b << bpp * 2);

                const uint8_t rout = r * 255U / bit_mask;
                const uint8_t gout = g * 255U / bit_mask;
                const uint8_t bout = b * 255U / bit_mask;

                palette[index] = SDL_MapRGB(app->pixel_format_details, NULL, rout ,gout ,bout);
            }
        }
    }
}

static void generate_sg_palette(App* app, uint32_t* palette) {
    // https://www.smspower.org/uploads/Development/sg1000.txt
    static const SDL_Color SG_COLOUR_TABLE[] = {
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
        const SDL_Color c = SG_COLOUR_TABLE[i];
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

static void core_vblank_callback(void* user, uint32_t overscan_colour) {
    App* app = user;

    SDL_LockMutex(app->timer_shared_data.mutex);
        app->timer_shared_data.vblank_counter++;
    SDL_UnlockMutex(app->timer_shared_data.mutex);

    if (!app->rewind_counter) {
        app->rewind_counter = app->rewind_keyframe_interval;
        app->rewind_should_push = true;
    } else {
        app->rewind_counter--;
    }

    if (SMS_get_skip_frame(&app->sms)) {
        return;
    }

    app->pending_frame = true;
    app->overscan_colour = overscan_colour;
    app->pixel_buffer_index ^= 1;
    SMS_set_pixels(&app->sms, app->pixel_buffer[app->pixel_buffer_index ^ 1], SMS_SCREEN_WIDTH, app->pixel_format_details->bytes_per_pixel);
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
                input_set(app, down, SMS_Button_JOY1_LEFT);
            }
            else if (value > 0) {
                input_set(app, down, SMS_Button_JOY1_RIGHT);
            }
        }
        else if (axis == SDL_GAMEPAD_AXIS_LEFTY) {
            input_set(app, false, SMS_Button_JOY1_UP|SMS_Button_JOY1_DOWN);

            if (value < 0) {
                input_set(app, down, SMS_Button_JOY1_UP);
            }
            else if (value > 0) {
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
    struct AudioSharedData* shared_data = userdata;

    if (additional_amount) {
        SDL_Log("dropping samples: %d total: %d\n", additional_amount, total_amount);
    }

    SDL_AudioSpec spec;
    if (!SDL_GetAudioStreamFormat(stream, &spec, NULL)) {
        return;
    }

    // 1s worth of audio, 5th of second (83.3ms)
    // https://github.com/higan-emu/emulation-articles/tree/master/audio/dynamic-rate-control
    const double avail = SDL_GetAudioStreamAvailable(stream);
    const double ones = spec.freq * SDL_AUDIO_FRAMESIZE(spec);
    const double buf_size = ones / 5.0;
    const double freq = spec.freq;

    const double maxDelta = 0.005;
    const double fillLevel = (buf_size - avail) / buf_size;
    const double dynamicFrequency = ((1.0 - maxDelta) + 2.0 * fillLevel * maxDelta) * freq;
    const double ratio = SDL_clamp(freq / dynamicFrequency, 0.5, 2.0);

    const double speed = SPEED_TABLE[shared_data->speed_index];
    SDL_SetAudioStreamFrequencyRatio(stream, ratio * speed);
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

    if (!e->mod && e->down) {
        for (size_t i = 0; i < SDL_arraysize(REWIND_KEY_MAP); i++) {
            const struct RewindKeyMap* p = &REWIND_KEY_MAP[i];
            if (p->key == e->key) {
                rewind_bar_button(app, p->button);
            }
        }
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
    if (e->down) {
        for (size_t i = 0; i < SDL_arraysize(REWIND_GAMEPAD_BUTTON_MAP); i++) {
            const struct RewindGamepadButtonMap* p = &REWIND_GAMEPAD_BUTTON_MAP[i];
            if (p->key == e->button) {
                rewind_bar_button(app, p->button);
            }
        }
    }
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

static void on_rewind_toggle(App* app) {
    on_set_rewind(app, rewind_bar_enabled() ^ 1);
}

static void on_fullscreen_toggle(App* app) {
    const bool is_fullscreen = SDL_WINDOW_FULLSCREEN & SDL_GetWindowFlags(app->window);

    if (is_fullscreen) {
        SDL_ShowCursor();
    } else {
        SDL_HideCursor();
    }

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
    if (app->frame_blending) {
        // copy front buffer to previous texture.
        SDL_UpdateTexture(app->texture_previous, NULL, app->pixel_buffer[app->pixel_buffer_index], SMS_SCREEN_WIDTH * app->pixel_format_details->bytes_per_pixel);
        text_popup_push(TextPopupType_INFO, "Frame Blending: on");
    } else {
        text_popup_push(TextPopupType_INFO, "Frame Blending: off");
    }
}

static void on_speed_increase(App* app) {
    on_set_speed(app, app->speed_index + 1);
}

static void on_speed_decrease(App* app) {
    on_set_speed(app, app->speed_index - 1);
}

static void on_speed_reset(App* app) {
    on_set_speed(app, SPEED_DEFAULT_INDEX);
}

static void on_set_pause(App* app, bool enable) {
    if (enable != app->paused) {
        if (enable) {
            text_popup_push(TextPopupType_INFO, "Paused");
        } else {
            text_popup_push(TextPopupType_INFO, "Resumed");
        }

        app->paused = enable;
        on_update_sound_playback_state(app);
    }
}

static void on_set_rewind(App* app, bool enable) {
    if (enable != rewind_bar_enabled()) {
        if (enable) {
            text_popup_push(TextPopupType_INFO, "Rewinding");
        } else {
            text_popup_push(TextPopupType_INFO, "Resumed");
        }

        rewind_bar_set_open(app, enable);
        on_update_sound_playback_state(app);
    }
}

static void on_set_speed(App* app, int speed) {
    speed = SDL_clamp(speed, 0, SDL_arraysize(SPEED_TABLE) - 1);
    if (app->speed_index != speed) {
        app->speed_index = speed;
        on_speed_change(app);
    }
}

static void on_update_sound_playback_state(App* app) {
    if (should_emu_run(app)) {
        SDL_ResumeAudioStreamDevice(app->audio_stream);
    } else {
        SDL_PauseAudioStreamDevice(app->audio_stream);
    }
}

static void on_speed_change(App* app) {
    const float speed = SPEED_TABLE[app->speed_index];
    text_popup_push_arg(TextPopupType_INFO, "Speed %.2f", speed);

    // clear audio as we may go 8x -> 1x which would
    SDL_LockAudioStream(app->audio_stream);
        SDL_ClearAudioStream(app->audio_stream);
        SDL_SetAudioStreamFrequencyRatio(app->audio_stream, speed);
        app->audio_shared_data.speed_index = app->speed_index;
    SDL_UnlockAudioStream(app->audio_stream);
}

static bool should_emu_run(const App* app) {
    return mgb_has_rom() && !app->paused && app->focus && !rewind_bar_enabled();
}

void emulator_update_texture_pixels(App* app, const void* pixel_buffer) {
    app->pending_frame = false;

    SDL_Rect rect;
    SMS_get_pixel_region(&app->sms, &rect.x, &rect.y, &rect.w, &rect.h);

    const uint8_t bpp = app->pixel_format_details->bytes_per_pixel;
    const uint32_t src_pitch = SMS_SCREEN_WIDTH * bpp;
    const uint32_t pin_off = rect.y * src_pitch + rect.x * bpp;

    SDL_UpdateTexture(app->texture_current, &rect, (const uint8_t*)pixel_buffer + pin_off, src_pitch);
}

static void emulator_render(App* app) {
    if (!mgb_has_rom()) {
        return;
    }

    // update texture pixels if we have a new frame pending.
    if (app->pending_frame) {
        emulator_update_texture_pixels(app, app->pixel_buffer[app->pixel_buffer_index]);
    }

    // get the output size of the sms
    SDL_Rect rect;
    SMS_get_pixel_region(&app->sms, &rect.x, &rect.y, &rect.w, &rect.h);

    // get the output size of the sms
    const SDL_FRect src_rect = {.x = rect.x, .y = rect.y, .w = rect.w, .h = rect.h};

    if (app->frame_blending && !rewind_bar_enabled()) {
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
    SMS_run(&app->sms, cycles * SPEED_TABLE[app->speed_index]);
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
    return app->runahead.frames > 0 && app->speed_index == SPEED_DEFAULT_INDEX;
}

// clears frame count so that all new frames must be generated.
// this should be called on input change, loadstate and loadrom.
static void runahead_clear_frames(App* app) {
    app->runahead.count = 0;
}

// run the emulate for a single frame.
// will exit early if the emulate is paused or no rom etc.
// if runahead is disabled, then it will run a frame as normal.
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
    const size_t cycles = SDL_floor((double)SMS_cycles_per_frame(&app->sms) * delta);
    // const size_t cycles = SMS_CYCLES_PER_FRAME;

    if (!runahead_is_enabled(app)) {
        // run frame as normal.
        emulator_run(app, cycles, false, false, false);
        // clear frames here as speed change may have disable runahead.
        runahead_clear_frames(app);
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

static Uint32 sdl_timer_callback(void *userdata, SDL_TimerID timerID, Uint32 interval) {
    struct TimerSharedData* shared_data = userdata;

    SDL_LockMutex(shared_data->mutex);
        shared_data->vblank_fps = shared_data->vblank_counter;
        shared_data->gui_fps = shared_data->gui_counter;

        shared_data->vblank_counter = 0;
        shared_data->gui_counter = 0;
        shared_data->pending = true;
    SDL_UnlockMutex(shared_data->mutex);

    return SDL_MS_PER_SECOND; // re-schedule for 1s
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
        "    --ratio\n"
        "        0 - none           Stretched to the output resolution.\n"
        "        1 - auto           [Default] Sets the pixel ratio based on the system and region.\n\n"
        "        2 - ntsc           Sets the pixel ratio to 8:7.\n\n"
        "        3 - pal            Sets the pixel ratio to 2950000:2128137.\n\n"
        "        4 - gamegear       Sets the pixel ratio to 6:5.\n\n"
        "    --overscan_fill        Fills the screen with overscan colour rather than black boarders.\n"

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
    const char* patch_file = NULL;
    SDL_ScaleMode scaler = SDL_SCALEMODE_NEAREST;
    SDL_RendererLogicalPresentation stretch = SDL_LOGICAL_PRESENTATION_INTEGER_SCALE;
    int vsync = 1;
    int runahead = 0;
    bool frame_blending = false;
    bool fullscreen = false;
    bool loadstate = false;
    bool overscan_fill = true;

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
            case ArgsId_patch:
                patch_file = arg_data.value.s;
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
            case ArgsId_ratio:
                break;
            case ArgsId_overscan_fill:
                overscan_fill = arg_data.value.b;
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

    // const double ratio = 2950000.0 / 2128137.0;
    const double ratio = 8.0 / 7.0;
#ifdef EMSCRIPTEN
    app->sms_scale = 1;
    app->gg_scale = 1;
    // set window to be the entire size of display
    app->window_w = display_mode->w;
    app->window_h = display_mode->h;
#else
    const int scale = SDL_min(display_mode->w / SMS_SCREEN_WIDTH * ratio, display_mode->h / SMS_SCREEN_HEIGHT);
    app->sms_scale = scale > 1 ? scale - 1 : scale;
    app->gg_scale = 5;
    app->window_w = SMS_SCREEN_WIDTH * app->sms_scale * ratio;
    app->window_h = SMS_SCREEN_HEIGHT * app->sms_scale;
#endif

    app->frame_blending = frame_blending;
    app->overscan_fill = overscan_fill;
    app->quit = false;
    app->speed_index = SPEED_DEFAULT_INDEX;
    app->audio_shared_data.speed_index = app->speed_index;
    app->rewind_keyframe_interval = 90; // every 1.5s
    app->rewind_num_seconds = 60 * 5; // 5 minutes

    // setup video and textures
    const int window_flags = SDL_WINDOW_HIGH_PIXEL_DENSITY|SDL_WINDOW_RESIZABLE;
    if (!SDL_CreateWindowAndRenderer("TotalSMS", app->window_w, app->window_h, window_flags, &app->window, &app->renderer)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetWindowMinimumSize(app->window, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetRenderVSync(app->renderer, vsync)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetRenderLogicalPresentation(app->renderer, SMS_SCREEN_WIDTH * ratio, SMS_SCREEN_HEIGHT, stretch)) {
        return SDL_APP_FAILURE;
    }

    app->pixel_format = SDL_GetWindowPixelFormat(app->window);
    if (app->pixel_format == SDL_PIXELFORMAT_UNKNOWN) {
        return SDL_APP_FAILURE;
    }

    app->pixel_format_details = SDL_GetPixelFormatDetails(app->pixel_format);
    if (!app->pixel_format_details) {
        return SDL_APP_FAILURE;
    }

    app->pixel_buffer_size = app->pixel_format_details->bytes_per_pixel * SMS_SCREEN_WIDTH * SMS_SCREEN_HEIGHT;
    app->pixel_buffer[0] = SDL_calloc(1, app->pixel_buffer_size);
    app->pixel_buffer[1] = SDL_calloc(1, app->pixel_buffer_size);
    if (!app->pixel_buffer[0] || !app->pixel_buffer[1]) {
        return SDL_APP_FAILURE;
    }

    app->texture_current = SDL_CreateTexture(app->renderer, app->pixel_format, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    app->texture_previous = SDL_CreateTexture(app->renderer, app->pixel_format, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);
    if (!app->texture_current || !app->texture_previous) {
        return SDL_APP_FAILURE;
    }

    SDL_SetTextureScaleMode(app->texture_current, scaler);
    SDL_SetTextureScaleMode(app->texture_previous, scaler);

    app->audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL, sdl_audio_callback, &app->audio_shared_data);
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

    app->timer_shared_data.mutex = SDL_CreateMutex();
    if (!app->timer_shared_data.mutex) {
        return SDL_APP_FAILURE;
    }

    app->timer = SDL_AddTimer(SDL_MS_PER_SECOND, sdl_timer_callback, &app->timer_shared_data);
    if (!app->timer) {
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
    SMS_set_pixels(&app->sms, app->pixel_buffer[app->pixel_buffer_index ^ 1], SMS_SCREEN_WIDTH, app->pixel_format_details->bytes_per_pixel);
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

        FS.mount(IDBFS, { autoPersist: true }, "/save");
        FS.mount(IDBFS, { autoPersist: true }, "/state");

        FS.syncfs(true, function (err) {
            if (err) {
                console.log(err);
            }
        });
    );
#endif // EMSCRIPTEN

    if (bios_file && !mgb_load_bios_file(bios_file)) {
        return SDL_APP_FAILURE;
    }

    if (rom_file && !mgb_load_rom_file(rom_file)) {
        return SDL_APP_FAILURE;
    }

    if (loadstate && !mgb_load_state_file(NULL)) {
        return SDL_APP_FAILURE;
    }

    if (patch_file && !mgb_patch_rom_file(patch_file)) {
        return SDL_APP_FAILURE;
    }

    if (fullscreen && mgb_has_rom()) {
        on_fullscreen_toggle(app);
    }

    if (!mgb_has_rom()) {
        text_popup_push(TextPopupType_INFO, "Press CTRL+O to load ROM");
    }

    runahead_init(app, runahead);

    app->focus = SDL_GetWindowFlags(app->window) & SDL_WINDOW_INPUT_FOCUS;
    on_update_sound_playback_state(app);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    App* app = appstate;

    static Uint64 start = 0;
    static Uint64 now = 0;
    // const double TARGET_FRAME_TIME = 1.0 / 60;
    // pal
    // const double TARGET_FRAME_TIME = 1.0 / 49.701459;
    // ntsc
    const double TARGET_FRAME_TIME = 1.0 / SMS_target_fps(&app->sms);
    double delta = TARGET_FRAME_TIME;

    if (start == 0) {
        start = SDL_GetPerformanceCounter();
    }

    now = SDL_GetPerformanceCounter();
    delta = (double)(now - start) / (double)SDL_GetPerformanceFrequency();
    start = now;

    on_update_sound_playback_state(app);

    if (should_emu_run(app)) {
        runahead_run_frame(app, delta / TARGET_FRAME_TIME);

        if (app->rewind_should_push) {
            rewind_push_new_frame(app);
            app->rewind_should_push = false;
        }
    }

    // update window fps.
    bool second_elapsed = false;
    SDL_LockMutex(app->timer_shared_data.mutex);
        app->timer_shared_data.gui_counter++;

        if (app->timer_shared_data.pending) {
            second_elapsed = true;

            app->timer_shared_data.pending = false;
            char* str;
            if (0 < SDL_asprintf(&str, "TotalSMS | EMU: %d fps | GUI: %d fps", app->timer_shared_data.vblank_fps, app->timer_shared_data.gui_fps)) {
                SDL_SetWindowTitle(app->window, str);
                SDL_free(str);
            }
        }
    SDL_UnlockMutex(app->timer_shared_data.mutex);

    uint8_t r = 0, g = 0, b = 0, a = 255;
    if (app->overscan_fill) {
        SDL_GetRGBA(app->overscan_colour, app->pixel_format_details, NULL, &r, &g, &b, &a);
    }

    if (!SDL_SetRenderDrawColor(app->renderer, r, g, b, a)) {
        return SDL_APP_FAILURE;
    }

    if (!SDL_RenderClear(app->renderer)) {
        return SDL_APP_FAILURE;
    }

    emulator_render(app);
    rewind_bar_render(app);
    text_popup_render(app->renderer);

    if (!SDL_RenderPresent(app->renderer)) {
        return SDL_APP_FAILURE;
    }

    int vsync;
    if (!SDL_GetRenderVSync(app->renderer, &vsync)) {
        return SDL_APP_FAILURE;
    }

#ifdef EMSCRIPTEN
    if (second_elapsed) {
        flushsave();
    }
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
        if (app->rewind) {
            rewind_close(app->rewind);
        }

        if (app->rewind_buffer) {
            SDL_free(app->rewind_buffer);
        }

        text_popup_clear_all();
        runahead_exit(app);
        mgb_exit();
        SMS_quit(&app->sms);

        if (app->timer) {
            SDL_RemoveTimer(app->timer);
        }
        if (app->timer_shared_data.mutex) {
            SDL_DestroyMutex(app->timer_shared_data.mutex);
        }
        if (app->sample_data) {
            SDL_free(app->sample_data);
        }
        if (app->gamepad.pad) {
            SDL_CloseGamepad(app->gamepad.pad);
        }
        if (app->pixel_buffer[0]) {
            SDL_free(app->pixel_buffer[0]);
        }
        if (app->pixel_buffer[1]) {
            SDL_free(app->pixel_buffer[1]);
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
