#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"


bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size);

void SMS_step(struct SMS_Core* sms);
void SMS_run_frame(struct SMS_Core* sms);


// [Z80-MISC]
uint8_t Z80_get_8bit_general_register(const struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx);
uint16_t Z80_get_16bit_general_register(const struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx);

uint8_t Z80_get_8bit_special_register(const struct SMS_Core* sms, enum Z80_8bitSpecialRegisters idx);
uint16_t Z80_get_16bit_special_register(const struct SMS_Core* sms, enum Z80_16bitSpecialRegisters idx);

void Z80_set_8bit_general_register(struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx, uint8_t value);
void Z80_set_16bit_general_register(struct SMS_Core* sms, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx, uint16_t value);

void Z80_set_8bit_special_register(struct SMS_Core* sms, enum Z80_8bitSpecialRegisters idx, uint8_t value);
void Z80_set_16bit_special_register(struct SMS_Core* sms, enum Z80_16bitSpecialRegisters idx, uint16_t value);

#ifdef __cplusplus
}
#endif
