#include <sms.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <SDL.h>


static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;

static struct SMS_Core sms = {0};
static uint8_t ROM[SMS_ROM_SIZE_MAX] = {0};
static uint8_t scale = 2;


static bool read_file(const char* path, uint8_t* out_buf, size_t* out_size)
{
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0)
    {
        return false;
    }

    fread(out_buf, 1, size, f);
    *out_size = (size_t)size;
    fclose(f);

    return true;
}

static void core_on_vblank(void* user)
{
    (void)user;

    void* pixels; int pitch;

    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    memcpy(pixels, sms.vdp.pixels, sizeof(sms.vdp.pixels));
    SDL_UnlockTexture(texture);
}

static void cleanup()
{
    if (texture)    { SDL_DestroyTexture(texture); }
    if (renderer)   { SDL_DestroyRenderer(renderer); }
    if (window)     { SDL_DestroyWindow(window); }

    SDL_Quit();
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("missing args\n");
        goto fail;
    }

    size_t rom_size = 0;

    if (!read_file(argv[1], ROM, &rom_size))
    {
        printf("failed to read file %s\n", argv[1]);
        goto fail;
    }

    if (!SMS_init(&sms))
    {
        goto fail;
    }

    SMS_set_vblank_callback(&sms, core_on_vblank, NULL);

    if (!SMS_loadrom(&sms, ROM, rom_size))
    {
        printf("failed to load rom %s\n", argv[1]);
        goto fail;
    }

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SMS_SCREEN_WIDTH * scale, SMS_SCREEN_HEIGHT * scale, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB555, SDL_TEXTUREACCESS_STREAMING, SMS_SCREEN_WIDTH, SMS_SCREEN_HEIGHT);

    bool quit = false;

    while (!quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) quit = true;

            if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_ESCAPE: quit = true; break;
                }
            }
        }

        SMS_run_frame(&sms);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    cleanup();
    return 0;

fail:
    cleanup();
    return -1;
}
