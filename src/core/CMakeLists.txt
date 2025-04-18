cmake_minimum_required(VERSION 3.13.4)

project(SMS_Core LANGUAGES C)

# fetch libraries
include(FetchContent)

# apu
FetchContent_Declare(sn76489
    GIT_REPOSITORY https://github.com/ITotalJustice/sn76489.git
    GIT_TAG        v1.2.1
    GIT_PROGRESS   TRUE
)

# scheduler
FetchContent_Declare(scheduler
    GIT_REPOSITORY https://github.com/ITotalJustice/scheduler.git
    GIT_TAG        v1.1.0
    GIT_PROGRESS   TRUE
)

set(SN76489_CXX OFF)
FetchContent_MakeAvailable(sn76489 scheduler)

list(APPEND src_all
    sms_bus.c
    sms_z80.c
    sms_joypad.c
    sms.c
    sms_vdp.c
    sms_rom_database.c
    sms_savestate.c
)

list(APPEND src_single sms_single.c)

if (SMS_SINGLE_FILE)
    add_library(SMS_Core ${src_single})
else()
    add_library(SMS_Core ${src_all})
endif()

target_include_directories(SMS_Core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(SMS_Core PROPERTIES C_STANDARD 99)

target_add_common_cflags(SMS_Core PRIVATE)

# enable sanitizer_flags
if (SMS_DEV)
    set(SMS_DEBUG ON)

    list(APPEND sanitizer_flags
        -fsanitize=address
        -fsanitize=leak
        -fsanitize=undefined
        -fsanitize=pointer-compare
        -fsanitize=pointer-subtract
        -fstack-protector-all
    )

    target_add_compiler_flags(SMS_Core sanitizer_flags PUBLIC)
    target_add_linker_flags(SMS_Core sanitizer_flags PUBLIC)
endif()

if (DEFINED SMS_PIXEL_WIDTH)
    target_compile_definitions(SMS_Core PUBLIC SMS_PIXEL_WIDTH=${SMS_PIXEL_WIDTH})
endif()

target_compile_definitions(SMS_Core PRIVATE
    SMS_DEBUG=$<BOOL:${SMS_DEBUG}>
    SMS_ENABLE_FORCE_INLINE=$<BOOL:${SMS_FORCE_INLINE}>
    SMS_SINGLE_FILE=$<BOOL:${SMS_SINGLE_FILE}>
    SMS_FAST_AUDIO=$<BOOL:${SMS_FAST_AUDIO}>
)

target_link_libraries(SMS_Core PUBLIC sn76489 scheduler)
