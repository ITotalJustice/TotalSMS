#pragma once


#include "types.h"



#define SMS_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SMS_MAX(x, y) (((x) > (y)) ? (x) : (y))

// msvc prepro has a hard time with (macro && macro), so they have to be
// split into different if, else chains...
#if defined(__has_builtin)
#if __has_builtin(__builtin_expect)
#define LIKELY(c) (__builtin_expect(c,1))
#define UNLIKELY(c) (__builtin_expect(c,0))
#else
#define LIKELY(c) ((c))
#define UNLIKELY(c) ((c))
#endif // __has_builtin(__builtin_expect)
#else
#define LIKELY(c) ((c))
#define UNLIKELY(c) ((c))
#endif // __has_builtin

#if defined(__has_builtin)
#if __has_builtin(__builtin_unreachable)
#define SMS_UNREACHABLE(ret) __builtin_unreachable()
#else
#define SMS_UNREACHABLE(ret) return ret
#endif // __has_builtin(__builtin_unreachable)
#else
#define SMS_UNREACHABLE(ret) return ret
#endif // __has_builtin

// used mainly in debugging when i want to quickly silence
// the compiler about unsed vars.
#define SMS_UNUSED(var) ((void)(var))

// ONLY use this for C-arrays, not pointers, not structs
#define SMS_ARR_SIZE(array) (sizeof(array) / sizeof(array[0]))

// returns 1 OR 0
#define IS_BIT_SET(v, bit) (!!((v) & (bit)))

// clears the bit before setting
#define SET_BIT(v, bit, t) v = (v & ~(bit)) | ((t) << (bit))


uint8_t Z80_get_8bit_general_register(const struct Z80* z80, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx);
uint16_t Z80_get_16bit_general_register(const struct Z80* z80, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx);

uint8_t Z80_get_8bit_special_register(const struct Z80* z80, enum Z80_8bitSpecialRegisters idx);
uint16_t Z80_get_16bit_special_register(const struct Z80* z80, enum Z80_16bitSpecialRegisters idx);

void Z80_set_8bit_general_register(struct Z80* z80, enum Z80_RegisterSet set, enum Z80_8bitGeneralRegisters idx, uint8_t value);
void Z80_set_16bit_general_register(struct Z80* z80, enum Z80_RegisterSet set, enum Z80_16bitGeneralRegisters idx, uint16_t value);

void Z80_set_8bit_special_register(struct Z80* z80, enum Z80_8bitSpecialRegisters idx, uint8_t value);
void Z80_set_16bit_special_register(struct Z80* z80, enum Z80_16bitSpecialRegisters idx, uint16_t value);
