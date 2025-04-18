cmake_minimum_required(VERSION 3.18)

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)

# enable setting cmake options via set()
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

option(SDL3 "build sdl3 frontend" OFF)
option(LIBRETRO "build libretro core" OFF)

option(SMS_SINGLE_FILE "include all src in single.c" OFF)
option(SMS_DEBUG "enable debug" OFF)
option(SMS_DEV "enables debug and sanitizers" OFF)
option(SMS_FORCE_INLINE "enables force inline, disabled in debug mode" ON)
option(SMS_FAST_AUDIO "neighbour linear interpolation resampling of audio" OFF)

option(LTO "enable lto for all targets" OFF)

set(TotalSMS_VERSION_MAJOR 0)
set(TotalSMS_VERSION_MINOR 1)
set(TotalSMS_VERSION_PATCH 1)
set(TotalSMS_VERSION ${TotalSMS_VERSION_MAJOR}.${TotalSMS_VERSION_MINOR}.${TotalSMS_VERSION_PATCH})

project(TotalSMS
    VERSION ${TotalSMS_VERSION}
    DESCRIPTION "WIP SMS emulator!"
    HOMEPAGE_URL "https://github.com/ITotalJustice/TotalSMS"
    LANGUAGES C
)

message(STATUS "COMPILER: using ${CMAKE_C_COMPILER_ID}")

# write out compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "generating compile_commands.json to ${CMAKE_BINARY_DIR}/compile_commands.json")

if (LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if (ipo_supported)
        message(STATUS "IPO / LTO enabled for ALL targets")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "IPO / LTO not supported")
    endif()
endif()

if (SMS_DEV)
    set(SMS_DEBUG ON)
endif()

if (SDL3)
    set(PLATFORM_SDL3 ON)
endif()

if (LIBRETRO)
    if (SWITCH)
        set(BUILD_SHARED_LIBS OFF)
    else()
        set(BUILD_SHARED_LIBS ON)
    endif()
endif()

# emcmake sets this variable
if (EMSCRIPTEN)
    set(PLATFORM_EMSCRIPTEN ON)
endif()

add_subdirectory(src)
