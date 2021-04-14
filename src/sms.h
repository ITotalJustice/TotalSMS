#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include "types.h"


bool SMS_loadrom(struct SMS_Core* sms, const uint8_t* rom, size_t size);


#ifdef __cplusplus
}
#endif
