#include <sms.h>
#include <stdio.h>
#include <assert.h>


static struct SMS_Core sms = {0};
static uint8_t ROM[SMS_ROM_SIZE_MAX] = {0};


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

    if (!SMS_loadrom(&sms, ROM, rom_size))
    {
        printf("failed to load rom %s\n", argv[1]);
        return -1;
    }

    for (;;)
    {
        SMS_run_frame(&sms);
    }

    return 0;
}
