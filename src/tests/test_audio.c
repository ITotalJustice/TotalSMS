#include <sms.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <SDL.h>


enum
{
    SAMPLES = 1024,
    VOLUME = SDL_MIX_MAXVOLUME / 2,
    SDL_AUDIO_FREQ = 48000,
    SMS_AUDIO_FREQ = SDL_AUDIO_FREQ,
};


static struct SMS_Core sms = {0};
static uint8_t ROM[SMS_ROM_SIZE_MAX] = {0};

static SDL_AudioDeviceID audio_device = 0;


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

static void core_on_apu(void* user, struct SMS_ApuCallbackData* data)
{
    (void)user;

    // disable to allow for volume control
    #if 1
        const uint8_t sample = (data->tone0[0] + data->tone1[0] + data->tone2[0] + data->noise[0]);
    #else
        const uint8_t mixed_channels = (data->tone0[0] + data->tone1[0] + data->tone2[0] + data->noise[0]);
        uint8_t sample = 0;
        SDL_MixAudioFormat(&sample, (const uint8_t*)&mixed_channels, AUDIO_U8, sizeof(mixed_channels), VOLUME);
    #endif

    while ((SDL_GetQueuedAudioSize(audio_device)) > (1024 * 4))
    {
        SDL_Delay(4);
    }

    SDL_QueueAudio(audio_device, &sample, sizeof(sample));
}

static void cleanup()
{
    if (audio_device) { SDL_CloseAudioDevice(audio_device); }

    SDL_Quit();
}

int main(int argc, char const *argv[])
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

    SMS_set_apu_callback(&sms, core_on_apu, SMS_AUDIO_FREQ);
    SMS_set_better_drums(&sms, true);

    if (!SMS_loadrom(&sms, ROM, rom_size, -1))
    {
        printf("failed to load rom %s\n", argv[1]);
        goto fail;
    }

    // enable to record audio
    #if 0
        SDL_setenv("SDL_AUDIODRIVER", "disk", 1);
    #endif

    if (SDL_Init(SDL_INIT_AUDIO))
    {
        goto fail;
    }

    const SDL_AudioSpec wanted =
    {
        .freq = SDL_AUDIO_FREQ,
        .format = AUDIO_U8,
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

        SMS_run(&sms, SMS_CYCLES_PER_FRAME);
    }

    cleanup();
    return 0;

fail:
    cleanup();
    return -1;
}
