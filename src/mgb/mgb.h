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

bool mgb_init(struct SMS_Core* gb);
void mgb_exit(void);

void mgb_set_on_file_callback(void (*cb)(const char*, enum CallbackType, bool));

void mgb_set_save_folder(const char* path);
void mgb_set_rtc_folder(const char* path);
void mgb_set_state_folder(const char* path);

const char* mgb_get_save_folder(void);
const char* mgb_get_rtc_folder(void);
const char* mgb_get_state_folder(void);

// bool mgb_load_bios_filedialog(void);
bool mgb_load_bios_file(const char* path);
// bool mgb_load_bios_fd(int fd, bool own, const char* path);
// bool mgb_load_bios_data(const char* path, const uint8_t* data, size_t size);


bool mgb_load_rom_filedialog(void);
bool mgb_load_rom_file(const char* path);
bool mgb_load_rom_fd(int fd, bool own, const char* path);
bool mgb_load_rom_data(const char* path, const uint8_t* data, size_t size);

// setting the path=NULL will use the current rom_path
// to create the new path, eg, if rom_name = rom.bin
// then the output for a save will be rom.sav.
bool mgb_load_save_file(const char* path);
bool mgb_load_rtc_file(const char* path);
bool mgb_load_state_file(const char* path);

bool mgb_load_save_data(const uint8_t* data, size_t size);
bool mgb_load_rtc_data(const uint8_t* data, size_t size);
bool mgb_load_state_data(const uint8_t* data, size_t size);

bool mgb_load_state_filedialog(void);
bool mgb_save_state_filedialog(void);

bool mgb_save_save_file(const char* path);
bool mgb_save_rtc_file(const char* path);
bool mgb_save_state_file(const char* path);

// return true if rom is loaded
bool mgb_has_rom(void);

bool mgb_rewind_init(size_t seconds);
void mgb_rewind_close(void);

// save states and stores pixel data for that frame
bool mgb_rewind_push_frame(const void* pixels, size_t size);
// loads state and loads pixel data for that frame
bool mgb_rewind_pop_frame(void* pixels, size_t size);

#ifdef __cplusplus
}
#endif
