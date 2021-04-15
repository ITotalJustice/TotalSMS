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


#ifdef SMS_DEBUG
#include <stdio.h>
#include <assert.h>
#define SMS_log(...) fprintf(stdout, __VA_ARGS__)
#define SMS_log_err(...) fprintf(stderr, __VA_ARGS__)
#else
#define SMS_log(...)
#define SMS_log_err(...)
#endif // SMS_DEBUG


// [CPU]
void Z80_run(struct SMS_Core* sms);


// [BUS]
uint8_t SMS_read8(struct SMS_Core* sms, uint16_t addr);
void SMS_write8(struct SMS_Core* sms, uint16_t addr, uint8_t value);
uint16_t SMS_read16(struct SMS_Core* sms, uint16_t addr);
void SMS_write16(struct SMS_Core* sms, uint16_t addr, uint16_t value);

void sega_mapper_setup(struct SMS_Core* sms);
void codemaster_mapper_setup(struct SMS_Core* sms);
