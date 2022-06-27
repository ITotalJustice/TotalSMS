#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <ogcsys.h>
#include <fat.h>
#include <stdio.h>
#include <SDL.h>
#include <mgb.h>
#include <sms.h>

#define BUILT_IN_ROM (1)
#include "rom.h"
#define ROM roms_Sonic_The_Hedgehog__USA__Europe__sms
#define ROM_SIZE roms_Sonic_The_Hedgehog__USA__Europe__sms_len
#define ROM_NAME "rom.sms"

#define WINDOW_FLAGS SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN

#define FPS_SKIP (0)
#define DISABLE_SDL_RENDERING (0)
#define AUDIO_FREQ (48000)
#define AUDIO_START (1)
#define AUDIO_BUFFERS (6)
#define SKIP_VSYNC (1)
#define AUDIO_ENABLED (1)

// SOURCE-AUDIO: https://libogc.devkitpro.org/audio_8h.html
// audio code is based on MGBA wii port (thanks!)
// framebuffer code is based on the FB example from devkitpro gc examples!


#define SAMPLES (3840 * 2)

// docs recomend that the data is double buffered!
struct AudioBuffer
{
    uint16_t samples[SAMPLES] __attribute__((__aligned__(32)));
    volatile int size;
};

static volatile int audio_buffer_index = 0;
static volatile int audio_buffer_next_index = 0;
static struct AudioBuffer audio_buffers[AUDIO_BUFFERS] = {0};

static const int scale = 1;
static struct SMS_Core sms = {0};
static SDL_Surface* window = NULL;
static SDL_Surface* game_surface = NULL;
static int window_w = SMS_SCREEN_WIDTH*scale;
static int window_h = SMS_SCREEN_HEIGHT*scale;
static int window_bpp = 16;
static bool running = true;


static void render(void) {
    if (SDL_Flip(window)) {
        fprintf(stderr, "%s\n", SDL_GetError());
    }
}

static int surface_lock(SDL_Surface* surface) {
	if (SDL_MUSTLOCK(surface)) {
		return SDL_LockSurface(surface);
    }

    return 0; // no locking needed
}

static void surface_unlock(SDL_Surface* surface) {
	if (SDL_MUSTLOCK(surface)) {
		SDL_UnlockSurface(surface);
    }
}

static uint32_t core_colour_callback(void* user, uint8_t r, uint8_t g, uint8_t b) {
    if (SMS_is_system_type_gg(&sms)) {
        const uint8_t R = r << 4;
        const uint8_t G = g << 4;
        const uint8_t B = b << 4;

        return SDL_MapRGB(window->format, R, G, B);
    }
    else {
        const uint8_t R = r << 6;
        const uint8_t G = g << 6;
        const uint8_t B = b << 6;

        return SDL_MapRGB(window->format, R, G, B);
    }
}

static void memcpy2(Uint16* dst, const Uint16* src, int len) {
    while (len--) {
        *dst++ = *src++;
    }
}

static void slow_scale(Uint16* dst, const Uint16* src) {
    for (int y = 0; y < SMS_SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SMS_SCREEN_WIDTH; x++) {
            const Uint16 pixel = *src++;

            for (int i = 0; i < scale; i++) {
                *dst++ = pixel;
            }
        }

        for (int i = 0; i < scale-1; i++) {
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
    if (scale > 1) {
        surface_lock(window);
            slow_scale(window->pixels, game_surface->pixels);
        surface_unlock(window);
    }
    else {
        surface_unlock(game_surface);
            if (SDL_BlitSurface(game_surface, NULL, window, NULL)) {
                fprintf(stderr, "%s\n", SDL_GetError());
            }
        surface_lock(game_surface);
    }

    render();
#endif
}

// sdl events
static void OnQuitEvent(const SDL_QuitEvent* e) {
    (void)e;
    running = false;
}

static void OnActiveEvent(const SDL_ActiveEvent* e) {
	(void)e;
}

static void OnVideoResizeEvent(const SDL_ResizeEvent* e) {
	// in SDL1, we have to get a new video mode...
	window = SDL_SetVideoMode(
		e->w, e->h,
		window->format->BitsPerPixel,
		window->flags
	);
}

static void OnVideoExposeEvent(const SDL_ExposeEvent* e) {
	(void)e;
}

static void OnSysWMEvent(const SDL_SysWMEvent* e) {
    (void)e;
}

static void OnUserEvent(SDL_UserEvent* e) {
    (void)e;
}

static void scan_input(void) {
    PAD_ScanPads();
    const uint32_t buttons = PAD_ButtonsHeld(0);

    // this is a lazy way to set buttons.
    SMS_set_port_a(&sms, JOY1_B_BUTTON, buttons & PAD_BUTTON_B);
    SMS_set_port_a(&sms, JOY1_A_BUTTON, buttons & PAD_BUTTON_A);
    SMS_set_port_a(&sms, JOY1_UP_BUTTON, buttons & PAD_BUTTON_UP);
    SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, buttons & PAD_BUTTON_DOWN);
    SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, buttons & PAD_BUTTON_LEFT);
    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, buttons & PAD_BUTTON_RIGHT);
    SMS_set_port_b(&sms, PAUSE_BUTTON, buttons & PAD_BUTTON_START);
}

static void events(void) {
    scan_input();
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        switch (e.type) {
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

            case SDL_SYSWMEVENT:
                OnSysWMEvent(&e.syswm);
                break;

            case SDL_USEREVENT:
                OnUserEvent(&e.user);
                break;
        }
    }
}

static void cleanup(void) {
    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        SDL_CloseAudio();
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    if (SDL_WasInit(SDL_INIT_TIMER)) {
        SDL_QuitSubSystem(SDL_INIT_TIMER);
    }

    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        if (window) {
            window = NULL;
            // The framebuffer surface, or NULL if it fails.
            // The surface returned is freed by SDL_Quit()
            // and should nt be freed by the caller.
            // SOURCE: https://www.libsdl.org/release/SDL-1.2.15/docs/html/sdlsetvideomode.html
        }

        if (game_surface) {
            SDL_FreeSurface(game_surface);
            game_surface = NULL;
        }

        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    mgb_exit();
    SDL_Quit();
}

static void on_audio_dma()
{
    struct AudioBuffer* buffer = &audio_buffers[audio_buffer_index];

    if (buffer->size != SAMPLES)
    {
        return;
    }

    // we need to flush because we don't want it to use cached data
    DCFlushRange(buffer->samples, sizeof(buffer->samples));
    // this starts an audio dma transfer (addr, len)
    AUDIO_InitDMA((u32) buffer->samples, sizeof(buffer->samples));

    buffer->size = 0;
    audio_buffer_index = (audio_buffer_index + 1) % AUDIO_BUFFERS;
}

static void core_audio_callback(void* user, struct SMS_ApuCallbackData* data)
{
    struct AudioBuffer* buffer = &audio_buffers[audio_buffer_next_index];

    if (buffer->size == SAMPLES)
    {
        return;
    }

    buffer->samples[buffer->size++] = (data->tone0[0] + data->tone1[0] + data->tone2[0] + data->noise[0]) * 256;
    buffer->samples[buffer->size++] = (data->tone0[1] + data->tone1[1] + data->tone2[1] + data->noise[1]) * 256;

    if (buffer->size == SAMPLES)
    {
        audio_buffer_next_index = (audio_buffer_next_index + 1) % AUDIO_BUFFERS;

        if (!AUDIO_GetDMAEnableFlag())
        {
            on_audio_dma();
            AUDIO_StartDMA();
        }
    }
}

int main(int argc, char** argv)
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        goto fail;
    }

    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
    AUDIO_RegisterDMACallback(on_audio_dma);

    const SDL_VideoInfo* video_info = SDL_GetVideoInfo();

    window = SDL_SetVideoMode(window_w, window_h, window_bpp, WINDOW_FLAGS);
    if (!window) {
        goto fail;
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

    if (!game_surface) {
        goto fail;
    }

    SMS_init(&sms);
    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);
    SMS_set_apu_callback(&sms, core_audio_callback, AUDIO_FREQ);
    surface_lock(game_surface);
    SMS_set_pixels(&sms, game_surface->pixels, game_surface->w, game_surface->format->BytesPerPixel);

    mgb_init(&sms);
    #if BUILT_IN_ROM
    if (!mgb_load_rom_data(ROM_NAME, ROM, ROM_SIZE)) {
    #else
    if (!mgb_load_rom_file(argv[1])) {
    #endif
        goto fail;
    }

    while (running) {
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
