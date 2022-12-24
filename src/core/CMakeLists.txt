cmake_minimum_required(VERSION 3.13.4)

list(APPEND src_all
    sms_bus.c
    sms_z80.c
    sms_joypad.c
    sms.c
    sms_psg.c
    sms_vdp.c
    sms_rom_database.c
)

list(APPEND src_single sms_single.c)

if (PLATFORM_PSX)
    if (SMS_SINGLE_FILE)
        psn00bsdk_add_library(TotalSMS OBJECT ${src_single})
    else()
        psn00bsdk_add_library(TotalSMS OBJECT ${src_all})
    endif()
else()
    if (SMS_SINGLE_FILE)
        add_library(TotalSMS ${src_single})
    else()
        add_library(TotalSMS ${src_all})
    endif()
endif()

target_include_directories(TotalSMS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_add_common_cflags(TotalSMS PRIVATE)
target_apply_lto_in_release(TotalSMS)

# enable sanitizer_flags
if (SNES_DEV)
    set(SMS_DEBUG ON)

    list(APPEND sanitizer_flags
        -fsanitize=address
        -fsanitize=leak
        -fsanitize=undefined
        -fsanitize=pointer-compare
        -fsanitize=pointer-subtract
        -fstack-protector-all
    )

    add_compiler_flags(sanitizer_flags PUBLIC)
    add_linker_flags(sanitizer_flags PUBLIC)
endif()

if (DEFINED SMS_PIXEL_WIDTH)
    target_compile_definitions(TotalSMS PUBLIC SMS_PIXEL_WIDTH=${SMS_PIXEL_WIDTH})
endif()

target_compile_definitions(TotalSMS PRIVATE
    SMS_DEBUG=$<BOOL:${SMS_DEBUG}>
    SMS_ENABLE_FORCE_INLINE=$<BOOL:${SMS_FORCE_INLINE}>
    SMS_SINGLE_FILE=$<BOOL:${SMS_SINGLE_FILE}>
    SMS_DISBALE_AUDIO=$<BOOL:${SMS_DISBALE_AUDIO}>
)
