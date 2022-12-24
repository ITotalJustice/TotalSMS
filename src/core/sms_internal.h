#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sms_types.h"

// if neither set, check compiler, else, default to little
#if !defined(SMS_LITTLE_ENDIAN) && !defined(SMS_BIG_ENDIAN)
    #if defined(__BYTE_ORDER)
        #define SMS_LITTLE_ENDIAN (__BYTE_ORDER == __LITTLE_ENDIAN)
        #define SMS_BIG_ENDIAN (__BYTE_ORDER == __BIG_ENDIAN)
    #elif defined(__BYTE_ORDER__)
        #define SMS_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
        #define SMS_BIG_ENDIAN (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    #else
        #define SMS_LITTLE_ENDIAN (1)
        #define SMS_BIG_ENDIAN (0)
    #endif
#elif defined(SMS_LITTLE_ENDIAN) && !defined(SMS_BIG_ENDIAN)
    #define SMS_BIG_ENDIAN (!SMS_LITTLE_ENDIAN)
#elif !defined(SMS_LITTLE_ENDIAN) && defined(SMS_BIG_ENDIAN)
    #define SMS_LITTLE_ENDIAN (!SMS_BIG_ENDIAN)
#endif

#define SMS_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SMS_MAX(x, y) (((x) > (y)) ? (x) : (y))


#ifndef SMS_ENABLE_FORCE_INLINE
    #define SMS_ENABLE_FORCE_INLINE 1
#endif

#ifndef SMS_DISBALE_AUDIO
    #define SMS_DISBALE_AUDIO 0
#endif

#if SMS_ENABLE_FORCE_INLINE
    #if defined(_MSC_VER)
        #define FORCE_INLINE inline __forceinline
    #elif defined(__GNUC__)
        #define FORCE_INLINE inline __attribute__((always_inline))
    #elif defined(__clang__)
        #define FORCE_INLINE inline __attribute__((always_inline))
    #else
        #define FORCE_INLINE inline
    #endif
#else
    #define FORCE_INLINE inline
#endif

#if SMS_SINGLE_FILE
    #define SMS_STATIC static
    #define SMS_INLINE static inline
    #define SMS_FORCE_INLINE static FORCE_INLINE
#else
    #define SMS_STATIC
    #define SMS_INLINE
    #define SMS_FORCE_INLINE
#endif // SMS_SINGLE_FILE

#if defined __has_builtin
    #define HAS_BUILTIN(x) __has_builtin(x)
#else
    #if defined(__GNUC__)
        #define HAS_BUILTIN(x) (1)
    #else
        #define HAS_BUILTIN(x) (0)
    #endif
#endif // __has_builtin

#if HAS_BUILTIN(__builtin_expect)
    #define LIKELY(c) (__builtin_expect(c,1))
    #define UNLIKELY(c) (__builtin_expect(c,0))
#else
    #define LIKELY(c) ((c))
    #define UNLIKELY(c) ((c))
#endif // __has_builtin(__builtin_expect)

#if HAS_BUILTIN(__builtin_unreachable)
    #define UNREACHABLE(ret) __builtin_unreachable()
#else
    #define UNREACHABLE(ret) return ret
#endif // __has_builtin(__builtin_unreachable)

// used mainly in debugging when i want to quickly silence
// the compiler about unsed vars.
#define UNUSED(var) ((void)(var))

// ONLY use this for C-arrays, not pointers, not structs
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#if SMS_DEBUG
    #include <stdio.h>
    #include <assert.h>
    #define SMS_log(...) fprintf(stdout, __VA_ARGS__)
    #define SMS_log_err(...) fprintf(stderr, __VA_ARGS__)
    #define SMS_log_fatal(...) do { fprintf(stderr, __VA_ARGS__); assert(0); } while(0)
#else
    #define SMS_log(...)
    #define SMS_log_err(...)
    #define SMS_log_fatal(...)
#endif // SMS_DEBUG

// returns 1 OR 0
#define IS_BIT_SET(v, bit) (!!((v) & (1 << (bit))))

// [CPU]
SMS_STATIC void z80_init(struct SMS_Core* sms);
SMS_FORCE_INLINE void z80_run(struct SMS_Core* sms);
SMS_STATIC void z80_nmi(struct SMS_Core* sms);
SMS_STATIC void z80_irq(struct SMS_Core* sms);

// [BUS]
SMS_FORCE_INLINE uint8_t SMS_read8(struct SMS_Core* sms, uint16_t addr);
SMS_FORCE_INLINE void SMS_write8(struct SMS_Core* sms, uint16_t addr, uint8_t value);
SMS_FORCE_INLINE uint16_t SMS_read16(struct SMS_Core* sms, uint16_t addr);
SMS_FORCE_INLINE void SMS_write16(struct SMS_Core* sms, uint16_t addr, uint16_t value);

SMS_STATIC uint8_t SMS_read_io(struct SMS_Core* sms, uint8_t addr);
SMS_STATIC void SMS_write_io(struct SMS_Core* sms, uint8_t addr, uint8_t value);

SMS_STATIC void mapper_init(struct SMS_Core* sms);
SMS_STATIC void mapper_update(struct SMS_Core* sms);

// [APU]
SMS_INLINE void psg_reg_write(struct SMS_Core* sms, uint8_t value);
SMS_STATIC void psg_sync(struct SMS_Core* sms);
SMS_FORCE_INLINE void psg_run(struct SMS_Core* sms, uint8_t cycles);
SMS_STATIC void psg_init(struct SMS_Core* sms);

SMS_STATIC void vdp_init(struct SMS_Core* sms);
SMS_INLINE uint8_t vdp_status_flag_read(struct SMS_Core* sms);
SMS_INLINE void vdp_io_write(struct SMS_Core* sms, uint8_t addr, uint8_t value);
SMS_FORCE_INLINE bool vdp_has_interrupt(const struct SMS_Core* sms);
SMS_FORCE_INLINE void vdp_run(struct SMS_Core* sms, uint8_t cycles);

// [MISC]
SMS_STATIC bool SMS_has_bios(const struct SMS_Core* sms);
SMS_FORCE_INLINE bool SMS_parity16(uint16_t value);
SMS_FORCE_INLINE bool SMS_parity8(uint8_t value);
SMS_STATIC bool SMS_is_spiderman_int_hack_enabled(const struct SMS_Core* sms);
SMS_STATIC void vdp_mark_palette_dirty(struct SMS_Core* sms);

#ifdef __cplusplus
}
#endif
