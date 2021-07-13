#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"


SMSAPI bool SMS_init(struct SMS_Core* sms);

SMSAPI bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size);

SMSAPI void SMS_step(struct SMS_Core* sms);
SMSAPI void SMS_run_frame(struct SMS_Core* sms);
SMSAPI void SMS_run_frame_cycles(struct SMS_Core* sms, size_t cycles);
SMSAPI void SMS_run_frame_delta(struct SMS_Core* sms, double delta);

SMSAPI void SMS_set_pixels(struct SMS_Core* sms, void* pixels, uint32_t stride, uint8_t bpp);

SMSAPI void SMS_set_apu_callback(struct SMS_Core* sms, sms_apu_callback_t cb, void* user, uint32_t freq);
SMSAPI void SMS_set_vblank_callback(struct SMS_Core* sms, sms_vblank_callback_t cb, void* user);
SMSAPI void SMS_set_colour_callback(struct SMS_Core* sms, sms_colour_callback_t cb, void* user);

SMSAPI bool SMS_savestate(const struct SMS_Core* sms, struct SMS_State* state);
SMSAPI bool SMS_loadstate(struct SMS_Core* sms, const struct SMS_State* state);

SMSAPI void SMS_set_system_type(struct SMS_Core* sms, enum SMS_System system);
SMSAPI enum SMS_System SMS_get_system_type(const struct SMS_Core* sms);
SMSAPI bool SMS_is_system_type_gg(const struct SMS_Core* sms);

SMSAPI void SMS_get_pixel_region(const struct SMS_Core* sms, uint16_t* x, uint16_t* y, uint16_t* w, uint16_t* h);

SMSAPI void SMS_set_overscan_enable(struct SMS_Core* sms, bool enable);
SMSAPI bool SMS_is_overscan_enabled(const struct SMS_Core* sms);

// [INPUT]
SMSAPI void SMS_set_port_a(struct SMS_Core* sms, enum SMS_PortA pin, bool down);
SMSAPI void SMS_set_port_b(struct SMS_Core* sms, enum SMS_PortB pin, bool down);

#ifdef __cplusplus
}
#endif
