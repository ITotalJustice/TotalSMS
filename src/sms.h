#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"


SMSAPI bool SMS_init(struct SMS_Core* sms);

SMSAPI bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size);

SMSAPI void SMS_step(struct SMS_Core* sms);
SMSAPI void SMS_run_frame(struct SMS_Core* sms);

SMSAPI void SMS_set_apu_callback(struct SMS_Core* sms, sms_apu_callback_t cb, void* user, uint32_t freq);

// [INPUT]
SMSAPI void SMS_set_port_a(struct SMS_Core* sms, enum SMS_PortA pin, bool down);
SMSAPI void SMS_set_port_b(struct SMS_Core* sms, enum SMS_PortB pin, bool down);

// [Z80-MISC]
SMSAPI uint8_t Z80_get_8bit_general_register(const struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx);
SMSAPI uint16_t Z80_get_16bit_general_register(const struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx);

SMSAPI uint8_t Z80_get_8bit_special_register(const struct SMS_Core* sms, enum Z80_8bitSpecialRegisters idx);
SMSAPI uint16_t Z80_get_16bit_special_register(const struct SMS_Core* sms, enum Z80_16bitSpecialRegisters idx);

SMSAPI void Z80_set_8bit_general_register(struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx, uint8_t value);
SMSAPI void Z80_set_16bit_general_register(struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx, uint16_t value);

SMSAPI void Z80_set_8bit_special_register(struct SMS_Core* sms, enum Z80_8bitSpecialRegisters idx, uint8_t value);
SMSAPI void Z80_set_16bit_special_register(struct SMS_Core* sms, enum Z80_16bitSpecialRegisters idx, uint16_t value);

#ifdef __cplusplus
}
#endif
