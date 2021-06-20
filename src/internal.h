#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define CPU_CLOCK (3579545)
#define CYCLES_PER_FRAME (CPU_CLOCK / 60)


#define SMS_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SMS_MAX(x, y) (((x) > (y)) ? (x) : (y))


#if SMS_DEBUG
    #define FORCE_INLINE inline
#else
    #if defined(_MSC_VER)
        #define FORCE_INLINE inline __forceinline
    #elif defined(__GNUC__)
        #define FORCE_INLINE inline __attribute__((always_inline))
    #elif defined(__clang__)
        #define FORCE_INLINE inline __attribute__((always_inline))
    #else
        #define FORCE_INLINE inline
    #endif
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
    #define HAS_BUILTIN(x) (0)
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
    #define SMS_log(...) fprintf(stdout, __VA_ARGS__)
    #define SMS_log_err(...) fprintf(stderr, __VA_ARGS__)
    #define SMS_log_fatal(...) do { fprintf(stderr, __VA_ARGS__); assert(0); } while(0)
#else
    #define SMS_log(...)
    #define SMS_log_err(...)
    #define SMS_log_fatal(...)
#endif // SMS_DEBUG

// returns 1 OR 0
#define IS_BIT_SET(v, bit) (!!((v) & (bit)))

// clears the bit before setting
#define SET_BIT(v, bit, t) v = (v & ~(bit)) | ((t) << (bit))


// [CPU]
SMS_FORCE_INLINE void Z80_run(struct SMS_Core* sms);

SMS_STATIC void Z80_nmi(struct SMS_Core* sms);
SMS_STATIC void Z80_irq(struct SMS_Core* sms);

// [BUS]
SMS_FORCE_INLINE uint8_t SMS_read8(struct SMS_Core* sms, uint16_t addr);
SMS_FORCE_INLINE void SMS_write8(struct SMS_Core* sms, uint16_t addr, uint8_t value);
SMS_FORCE_INLINE uint16_t SMS_read16(struct SMS_Core* sms, uint16_t addr);
SMS_FORCE_INLINE void SMS_write16(struct SMS_Core* sms, uint16_t addr, uint16_t value);

SMS_INLINE uint8_t SMS_read_io(struct SMS_Core* sms, uint8_t addr);
SMS_INLINE void SMS_write_io(struct SMS_Core* sms, uint8_t addr, uint8_t value);

SMS_STATIC void sega_mapper_setup(struct SMS_Core* sms);
SMS_STATIC void codemaster_mapper_setup(struct SMS_Core* sms);

// [APU]
SMS_INLINE void SN76489_reg_write(struct SMS_Core* sms, uint8_t value);
SMS_FORCE_INLINE void SN76489_run(struct SMS_Core* sms, uint8_t cycles);
SMS_STATIC void SN76489_init(struct SMS_Core* sms);

SMS_INLINE uint8_t vdp_status_flag_read(struct SMS_Core* sms);
SMS_FORCE_INLINE void vdp_run(struct SMS_Core* sms, uint8_t cycles);

// [MISC]
SMS_FORCE_INLINE bool SMS_parity(unsigned value);

#ifdef __cplusplus
}
#endif
