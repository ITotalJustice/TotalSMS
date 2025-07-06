#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

enum CallbackType
{
    CallbackType_LOAD_ROM,
    CallbackType_LOAD_BIOS,
    CallbackType_LOAD_SAVE,
    CallbackType_LOAD_STATE,

    CallbackType_SAVE_SAVE,
    CallbackType_SAVE_STATE,

    CallbackType_PATCH_ROM,
};

// savestates compressed with defalte/inflate
// lz4 was considered, but it seemed silly to have 2 compression
// algorithims included in my code, although lz4 is arguably
struct StateMeta
{
    uint32_t magic;
    char platform_string[64]; // todo:
    uint32_t state_size;
    // if 0, then the state is uncompressed!
    uint32_t state_compressed_size;
    uint32_t padding;
    uint64_t timestamp;
    uint64_t playtime;
    uint8_t reserved[160];
};

enum { STATE_MAGIC = 0x536A0535 };

struct StateInfo
{
    struct StateMeta meta;
    uint8_t* png;
    size_t png_size;
};

#if 0
    #include <stdio.h>
    #define mgb_log(...) fprintf(stdout, __VA_ARGS__)
    #define mgb_log_err(...) fprintf(stderr, __VA_ARGS__)
#else
    #define mgb_log(...)
    #define mgb_log_err(...)
#endif

struct SMS_Core;

typedef void (*set_on_file_callback_func)(void *user, const char*, enum CallbackType, bool);
typedef void* (*convert_pixels_to_png_format_func)(void* user, int* out_w, int* out_h, int* out_channels);

bool mgb_init(struct SMS_Core* gb);
void mgb_exit(void);

void mgb_set_userdata(void* user);
void mgb_set_on_file_callback(set_on_file_callback_func cb);
void mgb_set_on_convert_pixels_to_png_format(convert_pixels_to_png_format_func cb);

void mgb_set_save_folder(const char* path);
void mgb_set_rtc_folder(const char* path);
void mgb_set_state_folder(const char* path);

const char* mgb_get_save_folder(void);
const char* mgb_get_rtc_folder(void);
const char* mgb_get_state_folder(void);

bool mgb_load_bios_file(const char* path);
// bool mgb_load_bios_fd(int fd, bool own, const char* path);
// bool mgb_load_bios_data(const char* path, const uint8_t* data, size_t size);


bool mgb_load_rom_file(const char* path);
bool mgb_load_rom_fd(int fd, bool own, const char* path);
bool mgb_load_rom_data(const char* path, const uint8_t* data, size_t size);

// setting the path=NULL will use the current rom_path
// to create the new path, eg, if rom_name = rom.bin
// then the output for a save will be rom.sav.
// bool mgb_load_save_file(const char* path);
// bool mgb_load_rtc_file(const char* path);
bool mgb_load_state_file(const char* path);

// bool mgb_load_save_data(const uint8_t* data, size_t size);
// bool mgb_load_rtc_data(const uint8_t* data, size_t size);
// bool mgb_load_state_data(const uint8_t* data, size_t size);

bool mgb_save_save_file(const char* path);
// bool mgb_save_rtc_file(const char* path);
bool mgb_save_state_file(const char* path);

bool mgb_patch_rom_file(const char* path);
bool mgb_patch_rom_fd(int fd, bool own, const char* path);
bool mgb_patch_rom_data(const char* path, const uint8_t* data, size_t size);

// return true if rom is loaded
bool mgb_has_rom(void);
const char* mgb_rom_path(void);

struct StateInfo* mgb_load_state_info_file(const char* path, bool load_png);
void mgb_free_state_info(struct StateInfo* info);

// set to -1 (default) to auto detect.
void mgb_set_region(int value);
void mgb_set_console(int value);
void mgb_set_system(int value);

#ifdef __cplusplus
}
#endif
