// this is a small-ish example of how you would use my SMS_core
// and how to write a basic "frontend".
#include <sms.h>

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

#ifdef EMSCRIPTEN
    #include <emscripten.h>
#endif


enum
{
    WIDTH = SMS_SCREEN_WIDTH,
    HEIGHT = SMS_SCREEN_HEIGHT,

    VOLUME = SDL_MIX_MAXVOLUME / 2,
    SAMPLES = 2048,
    SDL_AUDIO_FREQ = 96000,
};

enum TouchButtonID
{
    TouchButtonID_A,
    TouchButtonID_B,
    TouchButtonID_UP,
    TouchButtonID_DOWN,
    TouchButtonID_LEFT,
    TouchButtonID_RIGHT,
    TouchButtonID_FULLSCREEN,
    TouchButtonID_FILE,
};

static struct TouchButton
{
    const char* path;
    SDL_Texture* texture;
    int w, h;
    SDL_Rect rect;
} touch_buttons[] =
{
    [TouchButtonID_A] =
    {
        .path = "res/touch_buttons/a.bmp",
        .w = 40,
        .h = 40,
    },
    [TouchButtonID_B] =
    {
        .path = "res/touch_buttons/b.bmp",
        .w = 40,
        .h = 40,
    },
    [TouchButtonID_UP] =
    {
        .path = "res/touch_buttons/up.bmp",
        .w = 30,
        .h = 38,
    },
    [TouchButtonID_DOWN] =
    {
        .path = "res/touch_buttons/down.bmp",
        .w = 30,
        .h = 38,
    },
    [TouchButtonID_LEFT] =
    {
        .path = "res/touch_buttons/left.bmp",
        .w = 38,
        .h = 30,
    },
    [TouchButtonID_RIGHT] =
    {
        .path = "res/touch_buttons/right.bmp",
        .w = 38,
        .h = 30,
    },
    [TouchButtonID_FULLSCREEN] =
    {
        .path = "res/touch_buttons/fullscreen.bmp",
        .w = 48,
        .h = 48,
    },
    [TouchButtonID_FILE] =
    {
        .path = "res/touch_buttons/file.bmp",
        .w = 48,
        .h = 48,
    },
};


// bad name, basically it just keeps tracks of the multi touches
struct TouchCacheEntry
{
    SDL_FingerID id;
    enum TouchButtonID touch_id;
    bool down;
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))


static struct TouchCacheEntry touch_entries[8] = {0}; // max of 8 touches at once
static struct TouchCacheEntry mouse_entries[1] = {0}; // max of 1 mouse clicks at once

static struct SMS_Core sms;
static uint32_t core_pixels[HEIGHT][WIDTH];

static const char* rom_path = NULL;
static uint8_t rom_data[SMS_ROM_SIZE_MAX] = {0};
static size_t rom_size = 0;
static bool has_rom = false;

static bool is_mobile = false;
static bool running = true;
// for now, scale web version slightly larger, this is until i learn html css :)
#ifdef EMSCRIPTEN
    static int scale = 3;
#else
    static int scale = 2;
#endif
static int speed = 1;
static int frameskip_counter = 0;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static SDL_AudioDeviceID audio_device = 0;
static SDL_Rect rect = {0};
static SDL_PixelFormat* pixel_format = NULL;
static SDL_GameController* game_controller = NULL;


static void toggle_fullscreen();

#ifdef EMSCRIPTEN
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

static void filedialog()
{
    EM_ASM(
        let rom_input = document.getElementById("RomFilePicker");
        rom_input.click();
    );
}

EMSCRIPTEN_KEEPALIVE
void em_set_browser_type(bool _is_mobile)
{
    is_mobile = _is_mobile;
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
#endif // #ifdef EMSCRIPTEN

static int get_scale(int w, int h)
{
    const int scale_w = w / WIDTH;
    const int scale_h = h / HEIGHT;

    return scale_w < scale_h ? scale_w : scale_h;
}

static void on_touch_button_change(enum TouchButtonID touch_id, bool down)
{
    switch (touch_id)
    {
        case TouchButtonID_A:        SMS_set_port_a(&sms, JOY1_A_BUTTON, down);      break;
        case TouchButtonID_B:        SMS_set_port_a(&sms, JOY1_B_BUTTON, down);      break;
        case TouchButtonID_UP:       SMS_set_port_a(&sms, JOY1_UP_BUTTON, down);     break;
        case TouchButtonID_DOWN:     SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, down);   break;
        case TouchButtonID_LEFT:     SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, down);   break;
        case TouchButtonID_RIGHT:    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, down);  break;
        
        case TouchButtonID_FULLSCREEN:  if (down) { toggle_fullscreen(); }  break;
        case TouchButtonID_FILE:        break;
    }
}

static int is_touch_in_range(int x, int y)
{
    for (size_t i = 0; i < ARRAY_SIZE(touch_buttons); ++i)
    {
        const struct TouchButton* e = (const struct TouchButton*)&touch_buttons[i];

        if (x >= e->rect.x && x <= (e->rect.x + e->rect.w))
        {

            if (y >= e->rect.y && y <= (e->rect.y + e->rect.h))
            {
                return (int)i;
            }
        }
    }

    return -1;
}

static void on_touch_up(struct TouchCacheEntry* cache, size_t size, SDL_FingerID id)
{
    for (size_t i = 0; i < size; ++i)
    {
        if (cache[i].down && cache[i].id == id)
        {
            cache[i].down = false;
            on_touch_button_change(cache[i].touch_id, false);

            break;
        }
    }
}

static void on_touch_down(struct TouchCacheEntry* cache, size_t size, SDL_FingerID id, int x, int y)
{
    // check that the button press maps to a texture coord
    const int touch_id = is_touch_in_range(x, y);

    if (touch_id == -1)
    {
        return;
    }

    // find the first free entry and add it to it
    for (size_t i = 0; i < size; ++i)
    {
        if (cache[i].down == false)
        {
            cache[i].id = id;
            cache[i].touch_id = touch_id;
            cache[i].down = true;

            on_touch_button_change(cache[i].touch_id, true);

            break;
        }
    }
}

static void on_touch_motion(struct TouchCacheEntry* cache, size_t size, SDL_FingerID id, int x, int y)
{
    // check that the button press maps to a texture coord
    const int touch_id = is_touch_in_range(x, y);

    if (touch_id == -1)
    {
        return;
    }

    // this is pretty inefficient, but its simple enough and works.
    on_touch_up(cache, size, id);
    on_touch_down(cache, size, id, x, y);
}

static void run()
{
    for (int i = 0; i < speed; ++i)
    {
        SMS_run_frame(&sms);
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
    if (!has_rom)
    {
        return;
    }

    struct SMS_State state;
    
    #ifdef EMSCRIPTEN
        char path[0x304] = {"/states/"};
    #else
        char path[0x304] = {0};
    #endif

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

        #ifdef EMSCRIPTEN
            syncfs();
        #endif
    }
}

static void loadstate()
{
    if (!has_rom)
    {
        return;
    }
    
    struct SMS_State state;
    
    #ifdef EMSCRIPTEN
        char path[0x304] = {"/states/"};
    #else
        char path[0x304] = {0};
    #endif

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

static void resize_touch_buttons(int w, int h)
{
    const int min_scale = get_scale(w, h);

    if (is_mobile)
    {
        touch_buttons[TouchButtonID_A].rect.x = w - 100 * min_scale;
        touch_buttons[TouchButtonID_A].rect.y = h - 50 * min_scale;
        touch_buttons[TouchButtonID_A].rect.w = touch_buttons[TouchButtonID_A].w * min_scale;
        touch_buttons[TouchButtonID_A].rect.h = touch_buttons[TouchButtonID_A].h * min_scale;

        touch_buttons[TouchButtonID_B].rect.x = w - 50 * min_scale;
        touch_buttons[TouchButtonID_B].rect.y = h - 50 * min_scale;
        touch_buttons[TouchButtonID_B].rect.w = touch_buttons[TouchButtonID_B].w * min_scale;
        touch_buttons[TouchButtonID_B].rect.h = touch_buttons[TouchButtonID_B].h * min_scale;

        touch_buttons[TouchButtonID_UP].rect.x = 35 * min_scale;
        touch_buttons[TouchButtonID_UP].rect.y = h - 90 * min_scale;
        touch_buttons[TouchButtonID_UP].rect.w = touch_buttons[TouchButtonID_UP].w * min_scale;
        touch_buttons[TouchButtonID_UP].rect.h = touch_buttons[TouchButtonID_UP].h * min_scale;

        touch_buttons[TouchButtonID_DOWN].rect.x = 35 * min_scale;
        touch_buttons[TouchButtonID_DOWN].rect.y = h - 50 * min_scale;
        touch_buttons[TouchButtonID_DOWN].rect.w = touch_buttons[TouchButtonID_DOWN].w * min_scale;
        touch_buttons[TouchButtonID_DOWN].rect.h = touch_buttons[TouchButtonID_DOWN].h * min_scale;

        touch_buttons[TouchButtonID_LEFT].rect.x = 5 * min_scale;
        touch_buttons[TouchButtonID_LEFT].rect.y = h - 68 * min_scale;
        touch_buttons[TouchButtonID_LEFT].rect.w = touch_buttons[TouchButtonID_LEFT].w * min_scale;
        touch_buttons[TouchButtonID_LEFT].rect.h = touch_buttons[TouchButtonID_LEFT].h * min_scale;

        touch_buttons[TouchButtonID_RIGHT].rect.x = 56 * min_scale;
        touch_buttons[TouchButtonID_RIGHT].rect.y = h - 68 * min_scale;
        touch_buttons[TouchButtonID_RIGHT].rect.w = touch_buttons[TouchButtonID_RIGHT].w * min_scale;
        touch_buttons[TouchButtonID_RIGHT].rect.h = touch_buttons[TouchButtonID_RIGHT].h * min_scale;
    }
    else
    {
        touch_buttons[TouchButtonID_A].rect.x = -8000;
        touch_buttons[TouchButtonID_A].rect.y = -8000;
        touch_buttons[TouchButtonID_B].rect.x = -8000;
        touch_buttons[TouchButtonID_B].rect.y = -8000;
        touch_buttons[TouchButtonID_UP].rect.x = -8000;
        touch_buttons[TouchButtonID_UP].rect.y = -8000;
        touch_buttons[TouchButtonID_DOWN].rect.x = -8000;
        touch_buttons[TouchButtonID_DOWN].rect.y = -8000;
        touch_buttons[TouchButtonID_LEFT].rect.x = -8000;
        touch_buttons[TouchButtonID_LEFT].rect.y = -8000;
        touch_buttons[TouchButtonID_RIGHT].rect.x = -8000;
        touch_buttons[TouchButtonID_RIGHT].rect.y = -8000;
    }

    touch_buttons[TouchButtonID_FULLSCREEN].rect.x = w - 56 * min_scale;
    touch_buttons[TouchButtonID_FULLSCREEN].rect.y = 5 * min_scale;
    touch_buttons[TouchButtonID_FULLSCREEN].rect.w = touch_buttons[TouchButtonID_FULLSCREEN].w * min_scale;
    touch_buttons[TouchButtonID_FULLSCREEN].rect.h = touch_buttons[TouchButtonID_FULLSCREEN].h * min_scale;

    touch_buttons[TouchButtonID_FILE].rect.x = 5 * min_scale;
    touch_buttons[TouchButtonID_FILE].rect.y = 5 * min_scale;
    touch_buttons[TouchButtonID_FILE].rect.w = touch_buttons[TouchButtonID_FILE].w * min_scale;
    touch_buttons[TouchButtonID_FILE].rect.h = touch_buttons[TouchButtonID_FILE].h * min_scale;
}

static void setup_rect(int w, int h)
{
    const int min_scale = get_scale(w, h);

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
        // iirc fullscreen option only works on ios
        if (is_mobile)
        {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
        }
        else
        {
            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
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

        #ifdef EMSCRIPTEN
            case SDL_SCANCODE_O:
                filedialog();
                break;
        #endif

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
    
    #ifndef EMSCRIPTEN
        case SDL_SCANCODE_ESCAPE:
            running = false;
            break;
    #endif // EMSCRIPTEN

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
        case SDL_CONTROLLER_BUTTON_LEFTSTICK:       break;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:   break;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK:      break;
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

static void on_touch_event(const SDL_TouchFingerEvent* e)
{
    int win_w = 0, win_h = 0;

    SDL_GetRendererOutputSize(renderer, &win_w, &win_h);

    // we need to un-normalise x, y
    const int x = e->x * win_w;
    const int y = e->y * win_h;

    switch (e->type)
    {
        case SDL_FINGERUP:
            on_touch_up(touch_entries, ARRAY_SIZE(touch_entries), e->fingerId);
            break;

        case SDL_FINGERDOWN:
            on_touch_down(touch_entries, ARRAY_SIZE(touch_entries), e->fingerId, x, y);
            break;

        case SDL_FINGERMOTION:
            on_touch_motion(touch_entries, ARRAY_SIZE(touch_entries), e->fingerId, x, y);
            break;
    }
}

static void on_mouse_button_event(const SDL_MouseButtonEvent* e)
{
    // we already handle touch events...
    if (e->which == SDL_TOUCH_MOUSEID)
    {
        return;
    }

    switch (e->type)
    {
        case SDL_MOUSEBUTTONUP:
            on_touch_up(mouse_entries, ARRAY_SIZE(mouse_entries), e->which);
            break;

        case SDL_MOUSEBUTTONDOWN:
            on_touch_down(mouse_entries, ARRAY_SIZE(mouse_entries), e->which, e->x, e->y);
            break;
    }
}

static void on_mouse_motion_event(const SDL_MouseMotionEvent* e)
{
    // we already handle touch events!
    if (e->which == SDL_TOUCH_MOUSEID)
    {
        return;
    }

    // only handle left clicks!
    if (e->state & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        on_touch_motion(mouse_entries, ARRAY_SIZE(mouse_entries), e->which, e->x, e->y);
    }
}

static void on_window_event(const SDL_WindowEvent* e)
{
    switch (e->event)
    {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        {
            // use this rather than window size because iirc i had issues with
            // hi-dpi screens.
            int w = 0, h = 0;
            SDL_GetRendererOutputSize(renderer, &w, &h);

            setup_rect(w, h);
            resize_touch_buttons(w, h);
        }   break;
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
                running = false;
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

            case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:
                on_mouse_button_event(&e.button);
                break;

             case SDL_MOUSEMOTION:
                on_mouse_motion_event(&e.motion);
                break;

            case SDL_FINGERDOWN:
            case SDL_FINGERUP:
            case SDL_FINGERMOTION:
                on_touch_event(&e.tfinger);
                break;
        
            case SDL_WINDOWEVENT:
                on_window_event(&e.window);
                break;
        }
    } 
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

    #ifdef EMSCRIPTEN
        if (SDL_GetQueuedAudioSize(audio_device) > sizeof(buffer) * 6)
        {
            return;
        }
    #endif

    buffer[buffer_count++] = data->tone0 + data->tone1 + data->tone2 + data->noise;

    if (buffer_count == sizeof(buffer))
    {
        buffer_count = 0;

        uint8_t samples[sizeof(buffer)] = {0};

        SDL_MixAudioFormat(samples, (const uint8_t*)buffer, AUDIO_S8, sizeof(buffer), VOLUME);

        #ifndef EMSCRIPTEN
            while (SDL_GetQueuedAudioSize(audio_device) > (sizeof(buffer) * 4))
            {
                SDL_Delay(4);
            }
        #endif

        SDL_QueueAudio(audio_device, samples, sizeof(samples));
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

static void load_touch_buttons()
{
    #ifndef EMSCRIPTEN
        return;
    #endif

    for (size_t i = 0; i < ARRAY_SIZE(touch_buttons); ++i)
    {
        // only load the controller buttons on mobile
        if (!is_mobile && (i != TouchButtonID_FULLSCREEN && i != TouchButtonID_FILE))
        {
            continue;
        }

        SDL_Surface* surface = SDL_LoadBMP(touch_buttons[i].path);

        if (surface)
        {
            touch_buttons[i].texture = SDL_CreateTextureFromSurface(renderer, surface);
            touch_buttons[i].rect.w = touch_buttons[i].w;
            touch_buttons[i].rect.h = touch_buttons[i].h;

            SDL_FreeSurface(surface);
        }
        else
        {
            printf("failed to load: %s\n", SDL_GetError());
        }
    }
}

static void render()
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    for (size_t i = 0; i < ARRAY_SIZE(touch_buttons); ++i)
    {
        if (touch_buttons[i].texture)
        {
            SDL_RenderCopy(renderer, touch_buttons[i].texture, NULL, &touch_buttons[i].rect);
        }
    }

    SDL_RenderPresent(renderer);
}

#ifdef EMSCRIPTEN
static void em_loop()
{
    events();
    run();
    render();
}
#endif // #ifdef EMSCRIPTEN

static void cleanup()
{
    if (pixel_format)   { SDL_free(pixel_format); }
    if (audio_device)   { SDL_CloseAudioDevice(audio_device); }
    if (texture)        { SDL_DestroyTexture(texture); }
    if (renderer)       { SDL_DestroyRenderer(renderer); }
    if (window)         { SDL_DestroyWindow(window); }

    SDL_Quit();
}

int main(int argc, char** argv)
{
    if (!SMS_init(&sms))
    {
        goto fail;
    }

    #ifdef EMSCRIPTEN
        EM_ASM(
            FS.mkdir("/saves"); FS.mount(IDBFS, {}, "/saves");
            FS.mkdir("/states"); FS.mount(IDBFS, {}, "/states");

            FS.syncfs(true, function (err) {
                if (err) {
                    console.log(err);
                }
            });

            if (IsMobileBrowser()) {
                console.log("is a mobile browser");
                _em_set_browser_type(true);
            } else {
                console.log("is NOT a mobile browser");
                _em_set_browser_type(false);
            }
        );
    #else
        if (argc < 2)
        {
            goto fail;
        }

        rom_path = argv[1];

        FILE* f = fopen(rom_path, "rb");

        if (!f)
        {
            goto fail;
        }

        rom_size = fread(rom_data, 1, sizeof(rom_data), f);

        fclose(f);

        if (!rom_size)
        {
            goto fail;
        }

        if (!SMS_loadrom(&sms, rom_data, rom_size))
        {
            printf("failed to loadrom\n");
            goto fail;
        }

        has_rom = true;
    #endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER))
    {
        goto fail;
    }

    if (SDL_GameControllerAddMappingsFromFile("res/controller_mapping/gamecontrollerdb.txt"))
    {
        printf("failed to open controllerdb file! %s\n", SDL_GetError());
    }

    window = SDL_CreateWindow("TotalSMS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * scale, HEIGHT * scale, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        goto fail;
    }

    // this doesn't seem to work on chromebook...
    SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT);

    const uint32_t pixel_format_enum = SDL_GetWindowPixelFormat(window);

    pixel_format = SDL_AllocFormat(pixel_format_enum);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        goto fail;
    }

    texture = SDL_CreateTexture(renderer, pixel_format_enum, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    if (!texture)
    {
        goto fail;
    }

    load_touch_buttons();

    {
        int w = 0, h = 0;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        setup_rect(w, h);
        resize_touch_buttons(w, h);
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

    SMS_set_apu_callback(&sms, core_on_apu, NULL, aspec_got.freq + 512);
    SMS_set_vblank_callback(&sms, core_on_vblank, NULL);
    SMS_set_colour_callback(&sms, core_on_colour, NULL);
    SMS_set_pixels(&sms, core_pixels, SMS_SCREEN_WIDTH, pixel_format->BitsPerPixel);

    #ifdef EMSCRIPTEN
        emscripten_set_main_loop(em_loop, 0, true);
    #else
        while (running)
        {
            events();
            run();
            render();
        }
    #endif

    cleanup();

    return 0;

fail:
    printf("fail %s\n", SDL_GetError());
    cleanup();

    return -1;
}
