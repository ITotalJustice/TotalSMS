#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sms_types.h"

// for frontends to detect if the game (maybe) uses sram:
// 1: on rom load, check if a .sav exits, if so, load it
// 2: on exit, call SMS_used_sram(), if true, write [game].sav

// todo: support bios loading
// add flag [bool has_bios]
// on bios load, reset all regs to 0x00
// on rom load, check if has bios, before resetting regs
// on rom map change, check if has bios AND if bios is mapped

// todo: remove this (needs mgb frontend changes)
struct SMS_State
{
    uint8_t data[58764];
};

SMS_API bool SMS_init(struct SMS_Core* sms);
SMS_API void SMS_quit(struct SMS_Core* sms);

SMS_API bool SMS_loadbios(struct SMS_Core* sms, const uint8_t* bios, size_t size);
SMS_API bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size, int system_hint);
SMS_API void SMS_run(struct SMS_Core* sms, size_t cycles);

SMS_API bool SMS_loadsave(struct SMS_Core* sms, const uint8_t* data, size_t size);
SMS_API bool SMS_used_sram(const struct SMS_Core* sms);

SMS_API bool SMS_get_skip_audio(const struct SMS_Core* sms);
SMS_API bool SMS_get_skip_frame(const struct SMS_Core* sms);

// skips next frame of audio. audio is still generated, but samples aren't
// read out...
SMS_API void SMS_skip_audio(struct SMS_Core* sms, bool enable);
// skips the next frame. sprites are still processed as it's needed
// for the stat line, but nothing is rendered
SMS_API void SMS_skip_frame(struct SMS_Core* sms, bool enable);

// default is 4, max is 32, increasing this reduces flicker in some
// games, such as flicky.
SMS_API void SMS_set_mode1_max_sprites(struct SMS_Core* sms, uint8_t value);
// sets the max sprites to be displayed
// default is 8, max is 64, increasing this can reduce flicker
// in some games, such as Altered Beast.
SMS_API void SMS_set_mode4_max_sprites(struct SMS_Core* sms, uint8_t value);

SMS_API void SMS_set_pixels(struct SMS_Core* sms, void* pixels, uint16_t stride, uint8_t bpp);
SMS_API void SMS_set_builtin_palette(struct SMS_Core* sms, const uint32_t palette[16]);
SMS_API void SMS_set_apu_callback(struct SMS_Core* sms, sms_apu_callback_t cb, uint32_t freq);
SMS_API void SMS_set_vblank_callback(struct SMS_Core* sms, sms_vblank_callback_t cb);
SMS_API void SMS_set_colour_callback(struct SMS_Core* sms, sms_colour_callback_t cb);
SMS_API void SMS_set_input_callback(struct SMS_Core* sms, sms_input_callback_t cb);
SMS_API void SMS_set_userdata(struct SMS_Core* sms, void* userdata);

SMS_API size_t SMS_get_state_size(void);
SMS_API bool SMS_savestate(const struct SMS_Core* sms, void* data, size_t size, bool fast);
SMS_API bool SMS_loadstate(struct SMS_Core* sms, const void* data, size_t size);

SMS_API void SMS_set_system_type(struct SMS_Core* sms, enum SMS_System system);
SMS_API enum SMS_System SMS_get_system_type(const struct SMS_Core* sms);
SMS_API bool SMS_is_system_type_sms(const struct SMS_Core* sms);
SMS_API bool SMS_is_system_type_gg(const struct SMS_Core* sms);
SMS_API bool SMS_is_system_type_sg(const struct SMS_Core* sms);

SMS_API void SMS_get_pixel_region(const struct SMS_Core* sms, int* x, int* y, int* w, int* h);

// [INPUT]
SMS_API void SMS_set_buttons(struct SMS_Core* sms, uint16_t buttons, bool down);

SMS_API uint32_t SMS_crc32(const void* data, size_t size);

#ifdef __cplusplus
}
#endif
