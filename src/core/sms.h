#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

// for frontends to detect if the game (maybe) uses sram:
// 1: on rom load, check if a .sav exits, if so, load it
// 2: on exit, call SMS_used_sram(), if true, write [game].sav

// todo: support bios loading
// add flag [bool has_bios]
// on bios load, reset all regs to 0x00
// on rom load, check if has bios, before resetting regs
// on rom map change, check if has bios AND if bios is mapped

SMSAPI bool SMS_init(struct SMS_Core* sms);
SMSAPI bool SMS_loadbios(struct SMS_Core* sms, const uint8_t* bios, size_t size);
SMSAPI bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size, int system_hint);
SMSAPI void SMS_run(struct SMS_Core* sms, size_t cycles);

SMSAPI bool SMS_loadsave(struct SMS_Core* sms, const uint8_t* data, size_t size);
SMSAPI bool SMS_used_sram(const struct SMS_Core* sms);

SMSAPI void SMS_skip_frame(struct SMS_Core* sms, bool enable);
SMSAPI void SMS_set_pixels(struct SMS_Core* sms, void* pixels, uint16_t pitch, uint8_t bpp);
SMSAPI void SMS_set_apu_callback(struct SMS_Core* sms, sms_apu_callback_t cb, uint32_t freq);
SMSAPI void SMS_set_vblank_callback(struct SMS_Core* sms, sms_vblank_callback_t cb);
SMSAPI void SMS_set_colour_callback(struct SMS_Core* sms, sms_colour_callback_t cb);
SMSAPI void SMS_set_userdata(struct SMS_Core* sms, void* userdata);

SMSAPI bool SMS_savestate(const struct SMS_Core* sms, struct SMS_State* state);
SMSAPI bool SMS_loadstate(struct SMS_Core* sms, const struct SMS_State* state);

// i had a bug in the noise channel which made the drums in all games sound *much*
// better, so much so, that i assumed other emulators emulated the noise channel wrong!
// however, after listening to real hw, the drums were in fact always that bad sounding.
// setting this to true will re-enable better drums!
SMSAPI void SMS_set_better_drums(struct SMS_Core* sms, bool enable);

SMSAPI void SMS_set_system_type(struct SMS_Core* sms, enum SMS_System system);
SMSAPI enum SMS_System SMS_get_system_type(const struct SMS_Core* sms);
SMSAPI bool SMS_is_system_type_sms(const struct SMS_Core* sms);
SMSAPI bool SMS_is_system_type_gg(const struct SMS_Core* sms);
SMSAPI bool SMS_is_system_type_sg(const struct SMS_Core* sms);

SMSAPI void SMS_get_pixel_region(const struct SMS_Core* sms, int* x, int* y, int* w, int* h);

// [INPUT]
SMSAPI void SMS_set_port_a(struct SMS_Core* sms, enum SMS_PortA pin, bool down);
SMSAPI void SMS_set_port_b(struct SMS_Core* sms, enum SMS_PortB pin, bool down);

SMSAPI uint32_t SMS_crc32(uint32_t crc, const void* data, size_t size);

#ifdef __cplusplus
}
#endif
