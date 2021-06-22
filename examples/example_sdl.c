// this is a small-ish example of how you would use my SMS_core
// and how to write a basic "frontend".
#include <sms.h>

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

enum
{
    WIDTH = SMS_SCREEN_WIDTH,
    HEIGHT = SMS_SCREEN_HEIGHT,

    VOLUME = SDL_MIX_MAXVOLUME / 2,
    SAMPLES = 1024,
    SDL_AUDIO_FREQ = 48000,
    SMS_AUDIO_FREQ = SDL_AUDIO_FREQ,

    AUDIO_SLEEP = 2,
};


static struct SMS_Core sms;
static uint32_t core_pixels[HEIGHT][WIDTH];
static const char* rom_path = NULL;
static uint8_t* rom_data = NULL;
static size_t rom_size = 0;
static bool running = true;
static int scale = 2;
static int speed = 1;
static int frameskip_counter = 0;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static SDL_AudioDeviceID audio_device = 0;
static SDL_Rect rect = {0};
static SDL_PixelFormat* pixel_format = NULL;


static void run()
{
    for (int i = 0; i < speed; ++i)
    {
        SMS_run_frame(&sms);
    }
}

static void get_state_path(char path_out[0x304])
{
    const char* ext = strrchr(rom_path, '.');

    strncat(path_out, rom_path, ext - rom_path);
    strcat(path_out, ".sav");
}

static void savestate()
{
    struct SMS_State state;
    char path[0x304] = {0};

    get_state_path(path);

    FILE* f = fopen(path, "wb");
    
    if (f)
    {
        SMS_savestate(&sms, &state);
        fwrite(&state, 1, sizeof(state), f);
        fclose(f);
    }
}

static void loadstate()
{
    struct SMS_State state;
    char path[0x304] = {0};

    get_state_path(path);

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

            // resets the game
            case SDL_SCANCODE_R:
                SMS_set_port_b(&sms, RESET_BUTTON, down);
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

    switch (e->keysym.scancode)
    {
        case SDL_SCANCODE_X:        SMS_set_port_a(&sms, JOY1_A_BUTTON, down);        break;
        case SDL_SCANCODE_Z:        SMS_set_port_a(&sms, JOY1_B_BUTTON, down);        break;
        case SDL_SCANCODE_UP:       SMS_set_port_a(&sms, JOY1_UP_BUTTON, down);       break;
        case SDL_SCANCODE_DOWN:     SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, down);     break;
        case SDL_SCANCODE_LEFT:     SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, down);     break;
        case SDL_SCANCODE_RIGHT:    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, down);    break;
    
        case SDL_SCANCODE_ESCAPE:
            running = false;
            break;

        default: break; // silence enum warning
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
                running = false;
                return;
        
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                on_key_event(&e.key);
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

    // if speedup is enabled, skip x many samples in order to not fill the
    // audio buffer!
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

    buffer[buffer_count++] = data->tone0 + data->tone1 + data->tone2 + data->noise;

    if (buffer_count == sizeof(buffer))
    {
        buffer_count = 0;

        uint8_t samples[sizeof(buffer)] = {0};

        SDL_MixAudioFormat(samples, (const uint8_t*)buffer, AUDIO_S8, sizeof(buffer), VOLUME);

        // enable this if sync with audio
        while (SDL_GetQueuedAudioSize(audio_device) > (sizeof(buffer) * 4))
        {
            SDL_Delay(4);
        }

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

static void render()
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_RenderPresent(renderer);
}

static void cleanup()
{
    if (pixel_format)   { SDL_free(pixel_format); }
    if (audio_device)   { SDL_CloseAudioDevice(audio_device); }
    if (rom_data)       { SDL_free(rom_data); }
    if (texture)        { SDL_DestroyTexture(texture); }
    if (renderer)       { SDL_DestroyRenderer(renderer); }
    if (window)         { SDL_DestroyWindow(window); }

    SDL_Quit();
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        goto fail;
    }

    rom_path = argv[1];

    // enable to record audio
    #if 0
        SDL_setenv("SDL_AUDIODRIVER", "disk", 1);
    #endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        goto fail;
    }

    window = SDL_CreateWindow("TotalSMS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * scale, HEIGHT * scale, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

    if (!window)
    {
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
        goto fail;
    }

    texture = SDL_CreateTexture(renderer, pixel_format_enum, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

    if (!texture)
    {
        goto fail;
    }

    setup_rect(WIDTH * scale, HEIGHT * scale);

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

    audio_device = SDL_OpenAudioDevice(NULL, 0, &wanted, &aspec_got, 0);

    if (audio_device == 0)
    {
        goto fail;
    }

    SDL_PauseAudioDevice(audio_device, 0);

    if (!SMS_init(&sms))
    {
        goto fail;
    }

    SMS_set_apu_callback(&sms, core_on_apu, NULL, SMS_AUDIO_FREQ);
    SMS_set_vblank_callback(&sms, core_on_vblank, NULL);
    SMS_set_colour_callback(&sms, core_on_colour, NULL);
    SMS_set_pixels(&sms, core_pixels, SMS_SCREEN_WIDTH, pixel_format->BitsPerPixel);

    rom_data = (uint8_t*)SDL_LoadFile(rom_path, &rom_size);

    if (!rom_data)
    {
        goto fail;
    }
    if (!SMS_loadrom(&sms, rom_data, rom_size))
    {
        printf("failed to loadrom\n");
        goto fail;
    }

    while (running)
    {
        events();
        run();
        render();
    }

    cleanup();

    return 0;

fail:
    printf("fail\n");
    printf("%s\n", SDL_GetError());
    cleanup();

    return -1;
}
