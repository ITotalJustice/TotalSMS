#include <SDL.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sms.h>
#include <mgb.h>

#if defined(PS2) || defined(__3DS__) || defined(DREAMCAST) || defined(__GAMECUBE__)
    #define BUILT_IN_ROM 1
#else
    #define BUILT_IN_ROM 0
#endif

#define FPS_SKIP (0)

// for testing on consoles if sdl is super slow
#define DISABLE_SDL_RENDERING (0)

#if BUILT_IN_ROM
    #include "rom.h"
    #define ROM roms_Sonic_The_Hedgehog__USA__Europe__sms
    #define ROM_SIZE roms_Sonic_The_Hedgehog__USA__Europe__sms_len
    #define ROM_NAME "rom.sms"
#endif

#define AUDIO_FREQ (48000)
#if defined(PS2)
    #define WINDOW_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN
#elif defined(__3DS__)
    #define WINDOW_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN
#elif defined(__GAMECUBE__)
    #define WINDOW_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN
#elif defined(DREAMCAST)
    #define WINDOW_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN
#else
    #define WINDOW_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE
#endif
#define SAMPLES 2048

struct AudioData
{
    Sint16 buffer[SAMPLES*2];
    Uint32 size;
};

#define AUDIO_ENTRIES 4

#if defined(PS2) || defined(__3DS__) || defined(DREAMCAST) || defined(__GAMECUBE__)
    static const int scale = 1;
#else
    static const int scale = 3;
#endif
static struct SMS_Core sms = {0};
static SDL_Surface* window = NULL;
static SDL_Surface* game_surface = NULL;
static SDL_Joystick* joystick = NULL;
static int window_w = SMS_SCREEN_WIDTH*scale;
static int window_h = SMS_SCREEN_HEIGHT*scale;
static int window_bpp = 16;
static struct AudioData audio_data[AUDIO_ENTRIES];
static struct SMS_ApuSample sms_audio_samples[SAMPLES];
static bool running = true;
static bool audio_init = false;


static void vsync(void)
{
    if (!(window->flags & SDL_DOUBLEBUF))
    {
        static int start_ticks = 0;
        static int end_ticks = 0;

        end_ticks = SDL_GetTicks();
        const int delay = (end_ticks - start_ticks);
        if (delay < 16)
        {
            SDL_Delay(16 - delay);
        }
        start_ticks = SDL_GetTicks();
    }
}

static void render(void)
{
    if (SDL_Flip(window))
    {
        fprintf(stderr, "%s\n", SDL_GetError());
    }

    vsync();
}

static int surface_lock(SDL_Surface* surface)
{
	if (SDL_MUSTLOCK(surface))
    {
		return SDL_LockSurface(surface);
    }

    return 0; // no locking needed
}

static void surface_unlock(SDL_Surface* surface)
{
	if (SDL_MUSTLOCK(surface))
    {
		SDL_UnlockSurface(surface);
    }
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
    if (SMS_is_system_type_gg(&sms))
    {
        const uint8_t R = r << 4;
        const uint8_t G = g << 4;
        const uint8_t B = b << 4;

        return SDL_MapRGB(window->format, R, G, B);
    }
    else
    {
        const uint8_t R = r << 6;
        const uint8_t G = g << 6;
        const uint8_t B = b << 6;

        return SDL_MapRGB(window->format, R, G, B);
    }
}

static void memcpy2(Uint16* dst, const Uint16* src, int len)
{
    while (len--)
    {
        *dst++ = *src++;
    }
}

static void slow_scale(Uint16* dst, const Uint16* src)
{
    for (int y = 0; y < SMS_SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SMS_SCREEN_WIDTH; x++)
        {
            const Uint16 pixel = *src++;

            for (int i = 0; i < scale; i++)
            {
                *dst++ = pixel;
            }
        }

        for (int i = 0; i < scale-1; i++)
        {
            memcpy2(dst, dst-SMS_SCREEN_WIDTH*scale, SMS_SCREEN_WIDTH*scale);
            dst+=SMS_SCREEN_WIDTH*scale;
        }
    }
}

static void core_vblank_callback(void* user)
{
    static int fps_skip_counter = FPS_SKIP;

    if (fps_skip_counter > 0)
    {
        fps_skip_counter--;
        SMS_skip_frame(&sms, true);
        return;
    }
    else
    {
        fps_skip_counter = FPS_SKIP;
        SMS_skip_frame(&sms, false);
    }

#if !DISABLE_SDL_RENDERING
    if (scale > 1)
    {
        surface_lock(window);
            slow_scale(window->pixels, game_surface->pixels);
        surface_unlock(window);
    }
    else
    {
        surface_unlock(game_surface);
            if (SDL_BlitSurface(game_surface, NULL, window, NULL))
            {
                fprintf(stderr, "%s\n", SDL_GetError());
            }
        surface_lock(game_surface);
    }

    render();
#endif
}

static void sdl_audio_callback(void* user, Uint8* data, int len)
{
    static int index = 0;

    if (len <= 0)
    {
        return;
    }

    if (audio_data[index].size < (Uint32)len/2)
    {
        memset(data, 0, len);
        return;
    }

    memcpy(data, audio_data[index].buffer, len);
    audio_data[index].size = 0;
    index = (index + 1) % AUDIO_ENTRIES;
}

// sdl events
static void OnQuitEvent(const SDL_QuitEvent* e)
{
    (void)e;
    running = false;
}

static void OnActiveEvent(const SDL_ActiveEvent* e)
{
	(void)e;
}

static void OnVideoResizeEvent(const SDL_ResizeEvent* e)
{
    printf("resize event\n");
	// in SDL1, we have to get a new video mode...
	window = SDL_SetVideoMode(
		e->w, e->h,
		window->format->BitsPerPixel,
		window->flags
	);
}

static void OnVideoExposeEvent(const SDL_ExposeEvent* e)
{
	(void)e;
}

static void OnMouseButtonEvent(const SDL_MouseButtonEvent* e)
{
}

static void OnMouseMotionEvent(const SDL_MouseMotionEvent* e)
{
}

static void OnKeyEvent(const SDL_KeyboardEvent* e)
{
    const bool down = e->type == SDL_KEYDOWN;

    switch (e->keysym.sym)
    {
        case SDLK_x:        SMS_set_port_a(&sms, JOY1_B_BUTTON, down);      break;
        case SDLK_z:        SMS_set_port_a(&sms, JOY1_A_BUTTON, down);      break;
        case SDLK_UP:       SMS_set_port_a(&sms, JOY1_UP_BUTTON, down);     break;
        case SDLK_DOWN:     SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, down);   break;
        case SDLK_LEFT:     SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, down);   break;
        case SDLK_RIGHT:    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, down);  break;
        case SDLK_r:        SMS_set_port_b(&sms, RESET_BUTTON, down);       break;
        case SDLK_RETURN:   SMS_set_port_b(&sms, PAUSE_BUTTON, down);       break;

        case SDLK_ESCAPE:
            running = false;
            break;

        default: break; // silence enum warning
    }
}

static void OnJoypadAxisEvent(const SDL_JoyAxisEvent* e)
{
    enum
    {
        JOY_AXIS_LEFT_X = 0,
        JOY_AXIS_LEFT_Y = 1,
        JOY_AXIS_RIGHT_X = 2,
        JOY_AXIS_RIGHT_Y = 3,
    };

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
        case JOY_AXIS_LEFT_X: case JOY_AXIS_RIGHT_X:
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
                // SMS_set_port_a(&sms, JOY1_XAXIS_BUTTON, false);
            }
            break;

        case JOY_AXIS_LEFT_Y: case JOY_AXIS_RIGHT_Y:
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
                // SMS_set_port_a(&sms, GB_BUTTON_YAXIS, false);
            }
            break;
    }

    // up AND right = positive
    printf("joy axis event axis: %u value: %d\n", e->axis, e->value);
}

static void OnJoypadButtonEvent(const SDL_JoyButtonEvent* e)
{
    const bool down = e->state == SDL_PRESSED;
#if defined(DREAMCAST)
    enum
    {
        JOY_A = 0,
        JOY_B = 1, //x
        JOY_Y = 3, //s
        JOY_X = 2, //a
        JOY_START = 4,

        JOY_L1, //d
        JOY_R1, //c
        JOY_SELECT,
        JOY_L2,
        JOY_R2,
        JOY_L3,
        JOY_R3,
    };
#elif defined(PS2)
    enum
    {
        JOY_A = 1,
        JOY_B = 2,
        JOY_Y = 3,
        JOY_X = 0,
        JOY_START = 5,
        JOY_SELECT = 4,
        JOY_L1 = 6,
        JOY_R1 = 7,
        JOY_L2 = 8,
        JOY_R2 = 9,
        JOY_L3 = 10,
        JOY_R3 = 11,
    };
#else
    // #error "unk joypad support!"
    #warning "unknown joypad support!"
    enum
    {
        JOY_A = 0,
        JOY_B = 1,
        JOY_Y = 3,
        JOY_X = 2,
        JOY_START = 5,
        JOY_SELECT = 4,
        JOY_L1 = 6,
        JOY_R1 = 7,
        JOY_L2 = 8,
        JOY_R2 = 9,
        JOY_L3 = 10,
        JOY_R3 = 11,
    };
#endif

    switch (e->button)
    {
        case JOY_A:
            SMS_set_port_a(&sms, JOY1_A_BUTTON, down);
            break;
        case JOY_B:
            SMS_set_port_a(&sms, JOY1_B_BUTTON, down);
            break;
        case JOY_START:
            SMS_set_port_b(&sms, PAUSE_BUTTON, down);
            break;

        case JOY_SELECT:
        case JOY_Y:
        case JOY_X:
        case JOY_L1:
        case JOY_R1:
        case JOY_L2:
        case JOY_R2:
        case JOY_L3:
        case JOY_R3:
            break;
    }
    printf("joy event button: %u\n", e->button);
}

static void OnJoypadHatEvent(const SDL_JoyHatEvent* e)
{
    if (e->value & SDL_HAT_UP)
    {
        SMS_set_port_a(&sms, JOY1_UP_BUTTON, true);
    }
    if (e->value & SDL_HAT_RIGHT)
    {
        SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, true);
    }
    if (e->value & SDL_HAT_DOWN)
    {
        SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, true);
    }
    if (e->value & SDL_HAT_LEFT)
    {
        SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, true);
    }
    if (e->value == SDL_HAT_CENTERED)
    {
        SMS_set_port_a(&sms, JOY1_UP_BUTTON, false);
        SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, false);
        SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, false);
        SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, false);
        // SMS_set_port_a(&sms, JOY1_DIRECTIONAL_BUTTON, false);
    }

    printf("joy hat event hat: %u value: %u\n", e->hat, e->value);
}

static void OnSysWMEvent(const SDL_SysWMEvent* e)
{
    (void)e;
}

static void OnUserEvent(SDL_UserEvent* e)
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
                OnQuitEvent(&e.quit);
                break;

            case SDL_ACTIVEEVENT:
                OnActiveEvent(&e.active);
                break;

            case SDL_VIDEORESIZE:
                OnVideoResizeEvent(&e.resize);
                break;

            case SDL_VIDEOEXPOSE:
                OnVideoExposeEvent(&e.expose);
                break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                OnMouseButtonEvent(&e.button);
                break;

            case SDL_MOUSEMOTION:
                OnMouseMotionEvent(&e.motion);
                break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
                OnKeyEvent(&e.key);
                break;

            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                OnJoypadButtonEvent(&e.jbutton);
                break;

            case SDL_JOYAXISMOTION:
                OnJoypadAxisEvent(&e.jaxis);
                break;

            case SDL_JOYHATMOTION:
                OnJoypadHatEvent(&e.jhat);
                break;

            case SDL_SYSWMEVENT:
                OnSysWMEvent(&e.syswm);
                break;

            case SDL_USEREVENT:
                OnUserEvent(&e.user);
                break;
        }
    }
}

static void cleanup(void)
{
    if (SDL_WasInit(SDL_INIT_JOYSTICK))
    {
        if (joystick)
        {
            SDL_JoystickClose(joystick);
            joystick = NULL;
        }
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
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
        if (window)
        {
            window = NULL;
            // The framebuffer surface, or NULL if it fails.
            // The surface returned is freed by SDL_Quit()
            // and should nt be freed by the caller.
            // SOURCE: https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlsetvideomode.html
        }

        if (game_surface)
        {
            SDL_FreeSurface(game_surface);
            game_surface = NULL;
        }

        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    mgb_exit();

    SDL_Quit();
}

#if defined(PS2)
#include <sifrpc.h>
#include <kernel.h>
#include <loadfile.h>
#include <iopheap.h>
#include <sbv_patches.h>

void PS2_init()
{
    // change priority to make SDL audio thread run properly
    ChangeThreadPriority(GetThreadId(), 72);

    // Initialize and connect to all SIF services on the IOP.
    SifInitRpc(0);
    SifInitIopHeap();
    SifLoadFileInit();
    // fioInit();

    // Apply the SBV LMB patch to allow modules to be loaded from a buffer in EE RAM.
    sbv_patch_enable_lmb();
}
#endif

#if defined(__3DS__)
#include <3ds.h>
static bool checkN3DS()
{
    bool isNew3DS = false;

    if (R_SUCCEEDED(APT_CheckNew3DS(&isNew3DS)))
        return isNew3DS;

    return false;
}
#endif

int main(int argc, char** argv)
{
    #if !BUILT_IN_ROM
    if (argc < 2)
    {
        return -1;
    }
    #endif

    #if defined(__3DS__)
    if (checkN3DS())
		osSetSpeedupEnable(false);
    #endif

    #if defined(PS2)
        PS2_init();
    #endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK))
    {
        goto fail;
    }

    // why does sdl enable this by default???
    SDL_ShowCursor(0);

    #if defined(__3DS__) || defined(PS2) || defined(DREAMCAST) || defined(__GAMECUBE__)
        joystick = SDL_JoystickOpen(0);
    #endif

    const SDL_VideoInfo* video_info = SDL_GetVideoInfo();

    // log the best video info
    printf("\nBest Video Info:\n");
        printf("\thw_available:\t%s\n", video_info->hw_available ? "TRUE" : "FALE");
        printf("\twm_available:\t%s\n", video_info->wm_available ? "TRUE" : "FALE");
        printf("\tblit_hw:\t%s\n", video_info->blit_hw ? "TRUE" : "FALE");
        printf("\tblit_hw_CC:\t%s\n", video_info->blit_hw_CC ? "TRUE" : "FALE");
        printf("\tblit_hw_A:\t%s\n", video_info->blit_hw_A ? "TRUE" : "FALE");
        printf("\tblit_sw:\t%s\n", video_info->blit_sw ? "TRUE" : "FALE");
        printf("\tblit_sw_CC:\t%s\n", video_info->blit_sw_CC ? "TRUE" : "FALE");
        printf("\tblit_sw_A:\t%s\n", video_info->blit_sw_A ? "TRUE" : "FALE");
        printf("\tblit_fill:\t%u\n", video_info->blit_fill);
        printf("\tvideo_mem:\t%u\n", video_info->video_mem);

    window = SDL_SetVideoMode(window_w, window_h, window_bpp, WINDOW_FLAGS);

    if (!window)
    {
        goto fail;
    }

    printf("window->w: %d\n", window->w);
    printf("window->h: %d\n", window->h);
    printf("window->pitch: %d\n", window->pitch);
    printf("window->format->BitsPerPixel: %d\n", window->format->BitsPerPixel);

    // check which flags where set!
    printf("\nWindow Surface Flags:\n");

    if (window->flags & SDL_SWSURFACE)
    {
        printf("\tSDL_SWSURFACE\n");
    }
    if (window->flags & SDL_HWSURFACE)
    {
        printf("\tSDL_OPENGL\n");
    }
    if (window->flags & SDL_ASYNCBLIT)
    {
        printf("\tSDL_ASYNCBLIT\n");
    }
    if (window->flags & SDL_HWPALETTE)
    {
        printf("\tSDL_HWPALETTE\n");
    }
    if (window->flags & SDL_DOUBLEBUF)
    {
        printf("\tSDL_DOUBLEBUF\n");
    }
    // if (window->flags & SDL_TRIPLEBUF)
    // {
    //     printf("\tSDL_TRIPLEBUF\n");
    // }
    if (window->flags & SDL_FULLSCREEN)
    {
        printf("\tSDL_FULLSCREEN\n");
    }
    if (window->flags & SDL_OPENGL)
    {
        printf("\tSDL_OPENGL\n");
    }
    if (window->flags & SDL_OPENGLBLIT)
    {
        printf("\tSDL_OPENGLBLIT\n");
    }
    if (window->flags & SDL_RESIZABLE)
    {
        printf("\tSDL_RESIZABLE\n");
    }
    if (window->flags & SDL_NOFRAME)
    {
        printf("\tSDL_NOFRAME\n");
    }

    game_surface = SDL_CreateRGBSurface(
    	SDL_SWSURFACE,
    	SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT,
        window->format->BitsPerPixel,
        window->format->Rmask,
        window->format->Gmask,
        window->format->Bmask,
        window->format->Amask
    );

    if (!game_surface)
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

    if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
    {
        audio_init = 0;
        return -1;
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
        SMS_set_better_drums(&sms, false);
    }
    surface_lock(game_surface);
    SMS_set_pixels(&sms, game_surface->pixels, game_surface->w, game_surface->format->BytesPerPixel);

    mgb_init(&sms);
    #if BUILT_IN_ROM
    if (!mgb_load_rom_data(ROM_NAME, ROM, ROM_SIZE))
    #else
    if (!mgb_load_rom_file(argv[1]))
    #endif
    {
        goto fail;
    }

    printf("loaded rom\n");

    while (running)
    {
        static int counter = 0;
        int start_time = SDL_GetTicks();

        events();
        SMS_run(&sms, SMS_CYCLES_PER_FRAME);

        counter++;
        if (counter == 60)
        {
            counter = 0;
            int frame_time = SDL_GetTicks() - start_time;
            double fps = (frame_time > 0) ? 1000.0 / (double)frame_time : 0.0;
            printf("FPS: %.2f\n", fps);
        }
    }

    cleanup();
    return 0;

fail:
    fprintf(stderr, "SDL FAIL: %s\n", SDL_GetError());
    cleanup();
    return -1;
}
