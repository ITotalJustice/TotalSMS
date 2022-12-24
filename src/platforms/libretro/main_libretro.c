#include "libretro.h"
#include "sms_types.h"
#include <sms.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define FPS 60
#define SAMPLE_RATE 48000


static struct SMS_Core sms = {0};

// double buffered
static uint16_t* framebuffers[2] = {0};
static uint8_t framebuffer_index = 0;
static uint16_t framebuffer_w = 0;
static uint16_t framebuffer_h = 0;

static uint8_t rom_data[SMS_ROM_SIZE_MAX] = {0};
static size_t rom_size = 0;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;


static uint32_t core_on_colour(void* user, uint8_t r, uint8_t g, uint8_t b)
{
    (void)user;

    if (SMS_is_system_type_gg(&sms))
    {
        // 12BPP
        return (r << 12) | (g << 7) | (b << 1);
    }
    else if (SMS_is_system_type_sms(&sms))
    {
        // 6BPP
        return (r << 14) | (g << 9) | (b << 3);
    }
    else // sg
    {
        return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    }
}

static void core_on_vblank(void* user)
{
    (void)user;

    framebuffer_index ^= 1;
    SMS_set_pixels(&sms, framebuffers[framebuffer_index], framebuffer_w, 16);
}

static void core_on_apu(void* user, struct SMS_ApuCallbackData* data)
{
    (void)user;

    int16_t left = data->tone0[0] + data->tone1[0] + data->tone2[0] + data->noise[0];
    int16_t right = data->tone0[1] + data->tone1[1] + data->tone2[1] + data->noise[1];

    audio_cb(left * 64, right * 64);
}

void retro_init(void)
{
    SMS_init(&sms);
    SMS_set_apu_callback(&sms, core_on_apu, 48000);
    SMS_set_vblank_callback(&sms, core_on_vblank);
    SMS_set_colour_callback(&sms, core_on_colour);
}

void retro_deinit(void)
{
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(struct retro_system_info));
    info->library_name = "TotalSMS";
    info->library_version = "0.0.1";
    info->valid_extensions = "sms|gg|zip";
    info->need_fullpath = false;
    info->block_extract = false;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    int x, y, w, h;
    SMS_get_pixel_region(&sms, &x, &y, &w, &h);

    w = SMS_SCREEN_WIDTH;
    h = SMS_SCREEN_HEIGHT;

    framebuffers[0] = calloc(w * h, sizeof(uint16_t));
    framebuffers[1] = calloc(w * h, sizeof(uint16_t));

    framebuffer_w = w;
    framebuffer_h = h;

    SMS_set_pixels(&sms, framebuffers[framebuffer_index], framebuffer_w, 16);

    info->timing.fps = FPS;
    info->timing.sample_rate = SAMPLE_RATE;

    info->geometry.aspect_ratio = (float)w / (float)h;
    info->geometry.base_width = w;
    info->geometry.base_height = h;
    info->geometry.max_width = w;
    info->geometry.max_height = h;

    enum retro_pixel_format pixel_format = RETRO_PIXEL_FORMAT_RGB565;
    environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &pixel_format);
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
}

// savestate size
size_t retro_serialize_size(void)
{
    return sizeof(struct SMS_State);
}

bool retro_serialize(void *data, size_t size)
{
    return SMS_savestate(&sms, (struct SMS_State*)data);
}

bool retro_unserialize(const void *data, size_t size)
{
    return SMS_loadstate(&sms, (const struct SMS_State*)data);
}

bool retro_load_game(const struct retro_game_info *game)
{
    // todo: support file loading
    if (game->size > SMS_ROM_SIZE_MAX || !game->size || !game->data)
    {
        return false;
    }

    // todo: parse path to see the type of game loaded
    // game->path

    memcpy(rom_data, game->data, game->size);
    rom_size = game->size;

    return SMS_loadrom(&sms, rom_data, rom_size, 0);
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
    // todo: support whatever this thing is
    return false;
}

void retro_unload_game(void)
{
    free(framebuffers[0]); framebuffers[0] = NULL;
    free(framebuffers[1]); framebuffers[1] = NULL;
}

// reset game
void retro_reset(void)
{
    SMS_loadrom(&sms, rom_data, rom_size, 0);
}

unsigned retro_get_region(void)
{
    return 0;
}

void *retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

enum
{
    DEVICE_ID_JOYPAD_B = 1 << 0,
    DEVICE_ID_JOYPAD_Y = 1 << 1,
    DEVICE_ID_JOYPAD_SELECT = 1 << 2,
    DEVICE_ID_JOYPAD_START = 1 << 3,
    DEVICE_ID_JOYPAD_UP = 1 << 4,
    DEVICE_ID_JOYPAD_DOWN = 1 << 5,
    DEVICE_ID_JOYPAD_LEFT = 1 << 6,
    DEVICE_ID_JOYPAD_RIGHT = 1 << 7,
    DEVICE_ID_JOYPAD_A = 1 << 8,
    DEVICE_ID_JOYPAD_X = 1 << 9,
    DEVICE_ID_JOYPAD_L = 1 << 10,
    DEVICE_ID_JOYPAD_R = 1 << 11,
    DEVICE_ID_JOYPAD_L2 = 1 << 12,
    DEVICE_ID_JOYPAD_R2 = 1 << 13,
    DEVICE_ID_JOYPAD_L3 = 1 << 14,
    DEVICE_ID_JOYPAD_R3 = 1 << 15,
};

void retro_run(void)
{
    // handle input
    input_poll_cb();

    uint16_t buttons = 0;

    // loop through all buttons on the controller
    for (uint8_t num_buttons = 0; num_buttons < 15; ++num_buttons)
    {
        // check which buttons are down
        if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, num_buttons))
        {
            // see above enum
            buttons |= 1 << num_buttons;
        }
    }

    SMS_set_port_a(&sms, JOY1_A_BUTTON, buttons & DEVICE_ID_JOYPAD_A);
    SMS_set_port_a(&sms, JOY1_B_BUTTON, buttons & DEVICE_ID_JOYPAD_B);
    SMS_set_port_a(&sms, JOY1_UP_BUTTON, buttons & DEVICE_ID_JOYPAD_UP);
    SMS_set_port_a(&sms, JOY1_DOWN_BUTTON, buttons & DEVICE_ID_JOYPAD_DOWN);
    SMS_set_port_a(&sms, JOY1_LEFT_BUTTON, buttons & DEVICE_ID_JOYPAD_LEFT);
    SMS_set_port_a(&sms, JOY1_RIGHT_BUTTON, buttons & DEVICE_ID_JOYPAD_RIGHT);
    SMS_set_port_b(&sms, PAUSE_BUTTON, buttons & DEVICE_ID_JOYPAD_START);

    // run for a frame
    SMS_run(&sms, SMS_CYCLES_PER_FRAME);

    // render
    video_cb(framebuffers[framebuffer_index ^ 1], framebuffer_w, framebuffer_h, sizeof(uint16_t) * framebuffer_w);
}
