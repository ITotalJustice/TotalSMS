#include "util.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
static const char dilem = '\\';
static const char* dilem_s = "\\";
#else
static const char dilem = '/';
static const char* dilem_s = "/";
#endif

// very basic case-insensitive compare
static int util_strcasecmp(const char* const a, const char* const b) {
    for (size_t i = 0; /* forever */ ; ++i) {
        // something is NULL, so we are at the end
        if (a[i] == '\0' || b[i] == '\0') {
            // if both are NULL, then we have a match!
            if (a[i] == b[i]) {
                return 0;
            }

            // otherwise, no match, return the corrrect value
            if (a[i] < b[i]) {
                return -1;
            }
            else {
                return +1;
            }
        }

        // no match, return correct value
        if (toupper(a[i]) != toupper(b[i])) {
            if (a[i] < b[i]) {
                return -1;
            }
            else {
                return +1;
            }
        }
    }
}

bool ss_valid(const struct SafeString* const ss) {
    return ss->str[0] != '\0';
}

struct SafeString ss_build(const char* a, ...) {
    struct SafeString ss = {0};
    va_list va = {0};
    va_start(va, a);
    vsnprintf(ss.str, sizeof(ss.str) - 1, a, va);
    va_end(va);
    return ss;
}

struct SafeString util_append_string(const char* a, const char* b) {
    struct SafeString ss = {0};
    strcpy(ss.str, a);
    strcat(ss.str, b);
    return ss;
}

struct SafeString util_append_extension(
    const char* str, const char* ext, enum ExtensionOffsetType type
) {
    struct SafeString ss = {0};

    if (!str || !ext) {
        goto fail;
    }

    size_t append_offset = 0;

    const size_t str_len = strlen(str);
    const size_t ext_len = strlen(ext);

    if (!str_len || !ext_len) {
        goto fail;
    }

    const long ext_offset = util_get_extension_offset(str, type);

    if (ext_offset <= 0) {
        append_offset = str_len;
    }
    else {
        append_offset = (size_t)ext_offset;
    }

    // check that this fits okay, +2 for '.' and at least 1 char after
    if (append_offset + 2 > sizeof(ss.str)) {
        goto fail;
    }

    strncpy(ss.str, str, append_offset);

    // check if the ext has the . prefix, if not, append it
    if (ext[0] != '.') {
        ss.str[append_offset] = '.';
        append_offset++;
    }

    // we have the final size now, check if it will all fit!
    if (append_offset + ext_len > sizeof(ss.str)) {
        goto fail;
    }

    strcpy(ss.str + append_offset, ext);

    return ss;

fail:
    ss.str[0] = '\0';
    return ss;
}

static struct SafeString util_test(const char* dir, const char* str, const char* ext)
{
    struct SafeString ss = {0};
    const char* rom_name = str;

    if (dir)
    {
        rom_name = strrchr(str, dilem);

        if (!rom_name)
        {
            rom_name = str;
        }
        else
        {
            rom_name++; // skip the ending '/'
        }

        strcpy(ss.str, dir);
        if (ss.str[strlen(ss.str)] != dilem)
        {
            strcat(ss.str, dilem_s);
        }
    }

    strcat(ss.str, rom_name);

    return util_append_extension(ss.str, ext, ExtensionOffsetType_LAST);
}

struct SafeString util_create_save_path(const char* dir, const char* str) {
    return util_test(dir, str, ".sav");
}

struct SafeString util_create_rtc_path(const char* dir, const char* str) {
    return util_test(dir, str, ".rtc");
}

struct SafeString util_create_state_path(const char* dir, const char* str) {
    return util_test(dir, str, ".state");
}

long util_get_extension_offset(
    const char* str, enum ExtensionOffsetType type
) {
    if (!str) {
        return -1;
    }

    const char* ext = NULL;

    // NOTE: this needs be a more sophisticated to handle
    // tar.xz files, and other multi-extension files.
    switch (type) {
        case ExtensionOffsetType_FIRST: ext = strchr(str, '.'); break;
        case ExtensionOffsetType_LAST: ext = strrchr(str, '.'); break;
    }

    if (!ext) {
        return -1;
    }

    const long difference = ext - str;

    // the ext we got might not be accurate, for example on android
    // saving to "org.sdl/saves/game" will actually give back the ext "org.sdl
    if (type == ExtensionOffsetType_LAST) {

        if (strchr(str + difference, dilem)) {
            return -1;
        }
    }

    // calculate the difference!
    return difference;
}

const char* util_get_extension(
    const char* str, enum ExtensionOffsetType type
) {
    const long offset = util_get_extension_offset(str, type);

    if (offset <= 0) {
        return NULL;
    }

    const size_t len = strlen(str + offset);

    if (len == 0) {
        return NULL;
    }

    return str + offset;
}

enum ExtensionType util_get_extension_type(
    const char* str, enum ExtensionOffsetType type
) {
    const char* ext = util_get_extension(str, type);

    if (!ext) {
        return ExtensionType_UNK;
    }

    static const struct ExtPair {
        const char* const ext;
        enum ExtensionType type;
    } ext_pairs[] = {
        // [roms]
        { ".sms", ExtensionType_SMS },
        { ".gg", ExtensionType_GG },
        { ".sg", ExtensionType_SG },

        // [zip]
        { ".zip", ExtensionType_ZIP },
        { ".gzip", ExtensionType_GZIP },
        { ".7zip", ExtensionType_7ZIP },
        { ".rar", ExtensionType_RAR },

        // [save]
        { ".save", ExtensionType_SAVE },
        { ".rtc", ExtensionType_RTC },
        { ".state", ExtensionType_STATE },

        // [patch]
        { ".ips", ExtensionType_IPS },
        { ".ups", ExtensionType_UPS },
        { ".bps", ExtensionType_BPS },
    };

    for (size_t i = 0; i < ARRAY_SIZE(ext_pairs); ++i) {
        if (!util_strcasecmp(ext, ext_pairs[i].ext)) {
            return ext_pairs[i].type;
        }
    }

    // no match!
    return ExtensionType_UNK;
}

bool util_is_extension(
    const char* str,
    enum ExtensionOffsetType type, enum ExtensionType wanted
) {
    return (wanted & util_get_extension_type(str, type)) > 0;
}
