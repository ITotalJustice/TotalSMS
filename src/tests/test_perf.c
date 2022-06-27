// this simply loads the rom
// give pixel data and apu callback
// so the mu will run as normal, just that callbacks
// will do nothing

#include <sms.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>


static struct SMS_Core sms = {0};
static uint8_t ROM[SMS_ROM_SIZE_MAX] = {0};
static uint16_t pixels[SMS_SCREEN_HEIGHT][SMS_SCREEN_WIDTH];

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

static void core_audio_callback(void* user, struct SMS_ApuCallbackData* data)
{
    (void)user; (void)data;
}

static uint32_t core_colour_callback(void* user, uint8_t r, uint8_t g, uint8_t b)
{
    (void)user; (void)r; (void)g; (void)b;
    return 0;
}

static void core_vblank_callback(void* user)
{
    (void)user;
}

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("missing args\n");
        return -1;
    }

    size_t rom_size = 0;

    if (!read_file(argv[1], ROM, &rom_size))
    {
        printf("failed to read file %s\n", argv[1]);
        return -1;
    }

    if (!SMS_init(&sms))
    {
        return -1;
    }

    if (!SMS_loadrom(&sms, ROM, rom_size, -1))
    {
        printf("failed to load rom %s\n", argv[1]);
        return -1;
    }

    SMS_set_colour_callback(&sms, core_colour_callback);
    SMS_set_vblank_callback(&sms, core_vblank_callback);
    SMS_set_apu_callback(&sms, core_audio_callback, 48000);
    SMS_set_pixels(&sms, pixels, SMS_SCREEN_WIDTH, sizeof(uint16_t));

    for (int i = 0; i < 10000; i++)
    {
        SMS_run(&sms, SMS_CYCLES_PER_FRAME);
    }

    return 0;
}
