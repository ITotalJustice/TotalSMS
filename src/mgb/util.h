#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>


// simple macro which silences warnings for stuff that i want to quickly ignore
#define UNUSED(x) (void)x

// same as above [UNUSED], but more clear that this var is yet to be implemented
#define STUB(x) (void)x

// only pass in c-arrays...
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


enum ExtensionType {
    ExtensionType_UNK   = 1 << 0,
    ExtensionType_SAVE  = 1 << 2,
    ExtensionType_RTC   = 1 << 3,
    ExtensionType_STATE = 1 << 4,
    ExtensionType_ZIP   = 1 << 5,
    ExtensionType_GZIP  = 1 << 6,
    ExtensionType_7ZIP  = 1 << 7,
    ExtensionType_RAR   = 1 << 8,
    ExtensionType_IPS   = 1 << 9,
    ExtensionType_UPS   = 1 << 10,
    ExtensionType_BPS   = 1 << 11,

    ExtensionType_SMS   = 1 << 1,
    ExtensionType_GG   = 1 << 2,
    ExtensionType_SG   = 1 << 3,
    ExtensionType_ROM = ExtensionType_SMS | ExtensionType_GG | ExtensionType_SG,

    ExtensionType_REGULAR = ExtensionType_UNK | ExtensionType_ROM |
        ExtensionType_SAVE | ExtensionType_RTC | ExtensionType_STATE |
        ExtensionType_IPS | ExtensionType_UPS | ExtensionType_BPS,

    ExtensionType_PATCH = ExtensionType_IPS | ExtensionType_UPS | ExtensionType_BPS,
};

enum ExtensionOffsetType {
    ExtensionOffsetType_FIRST,
    ExtensionOffsetType_LAST
};

#ifndef SAFE_STRING_SIZE
    #define SAFE_STRING_SIZE 0x304
#endif

// used to return strings from functions, without malloc / free
// thus clearing strings on the stack when out of scope.
struct SafeString {
    char str[SAFE_STRING_SIZE]; // set to false if the string is NULL
};

bool ss_valid(const struct SafeString* const ss);

// appends the standard extension to the given string
struct SafeString util_create_save_path(const char* dir, const char* str);
struct SafeString util_create_rtc_path(const char* dir, const char* str);
struct SafeString util_create_state_path(const char* dir, const char* str);

struct SafeString ss_build(const char* a, ...);
struct SafeString util_append_string(const char* a, const char* b);

// appends ext extension to str
struct SafeString util_append_extension(
    const char* str, const char* ext, enum ExtensionOffsetType type
);

// returns offset, -1 on error
long util_get_extension_offset(
    const char* str, enum ExtensionOffsetType type
);

// returns the extension
const char* util_get_extension(
    const char* str, enum ExtensionOffsetType type
);

// returns the type based on the extension.
// this is case-insensitive (.zip == .ZIP)
enum ExtensionType util_get_extension_type(
    const char* str, enum ExtensionOffsetType type
);

// simple [return util_get_extension_type() == wanted]
bool util_is_extension(
    const char* str,
    enum ExtensionOffsetType type, enum ExtensionType wanted
);

#ifdef __cplusplus
}
#endif
