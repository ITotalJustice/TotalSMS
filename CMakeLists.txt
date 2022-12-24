cmake_minimum_required(VERSION 3.18)

option(DREAMCAST "build dreamcast" OFF)
option(GAMECUBE "build gamecube" OFF)
option(N64 "build n64" OFF)
option(PSX "build psx" OFF)
option(PS2 "build ps2" OFF)
option(NDS "build nds" OFF)
option(3DS "build 3ds" OFF)
option(WII "build wii" OFF)
option(WIIU "build wiiu" OFF)
option(SWITCH "build switch" OFF)

option(SDL "build sdl frontend" OFF)
option(SDL2 "build sdl2 frontend" OFF)
option(LIBRETRO "build libretro core" OFF)

option(SMS_SINGLE_FILE "include all src in single.c" OFF)
option(SMS_DEBUG "enable debug" OFF)
option(SMS_DEV "enables debug and sanitizers" OFF)
option(SMS_FORCE_INLINE "enables force inline, disabled in debug mode" ON)
option(SMS_DISBALE_AUDIO "force disable of audio, useful for slow systems" OFF)

option(TEST_AUDIO "" OFF)
option(TEST_GFX "" OFF)
option(TEST_ROMRUNNER "" OFF)
option(TEST_PERF "" OFF)
option(TEST_ALL "build all tests" OFF)

option(LTO "enable lto for all targets" OFF)

macro(set_toolchain_for_system enabled path)
    if (${enabled})
        set(CMAKE_TOOLCHAIN_FILE ${path})
        set(BUILD_SHARED_LIBS OFF)
        set(SMS_SINGLE_FILE ON)
        set(CMAKE_BUILD_TYPE Release)
        message(STATUS "setting toolchain path to ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endmacro()

set_toolchain_for_system(${DREAMCAST} "cmake/toolchain/dreamcast.cmake")
set_toolchain_for_system(${GAMECUBE} "$ENV{DEVKITPRO}/cmake/GameCube.cmake")
set_toolchain_for_system(${N64} "cmake/toolchain/n64.cmake")
set_toolchain_for_system(${PSX} "/opt/psn00bsdk/lib/libpsn00b/cmake/sdk.cmake")
set_toolchain_for_system(${PS2} "cmake/toolchain/ps2.cmake")
set_toolchain_for_system(${NDS} "$ENV{DEVKITPRO}/cmake/NDS.cmake")
set_toolchain_for_system(${3DS} "$ENV{DEVKITPRO}/cmake/3DS.cmake")
set_toolchain_for_system(${WII} "$ENV{DEVKITPRO}/cmake/Wii.cmake")
set_toolchain_for_system(${WIIU} "$ENV{DEVKITPRO}/cmake/WiiU.cmake")
set_toolchain_for_system(${SWITCH} "$ENV{DEVKITPRO}/cmake/Switch.cmake")

project(TotalSMS
    VERSION 0.0.1
    DESCRIPTION "WIP SMS emulator!"
    HOMEPAGE_URL "https://github.com/ITotalJustice/TotalSMS"
    LANGUAGES C
)

message(STATUS "COMPILER: using ${CMAKE_C_COMPILER_ID}")

# write out compile commands in clangd is found
find_program(CLANGD_INSTALLED clangd)
if (CLANGD_INSTALLED)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    message(STATUS "found clangd: generating compile_commands.json to ${CMAKE_BINARY_DIR}/compile_commands.json")
else()
    message(STATUS "cant find clangd: not generating compile_commands.json")
endif()

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

if (TEST_ALL)
    set(TEST_AUDIO ON)
    set(TEST_GFX ON)
    set(TEST_ROMRUNNER ON)
endif()

if (SMS_DEV)
    set(SMS_DEBUG ON)
endif()

if (SDL)
    set(PLATFORM_SDL ON)
endif()

if (SDL2)
    set(PLATFORM_SDL2 ON)
endif()

if (NINTENDO_SWITCH)
    enable_language(CXX) # needed for linking sdl2
    set(PLATFORM_SDL2 ON)
    set(PLATFORM_SWITCH ON)
endif()

if (NINTENDO_GAMECUBE)
    set(PLATFORM_SDL ON)
    set(PLATFORM_GAMECUBE ON)
endif()

if (NINTENDO_DS)
    set(PLATFORM_NDS ON)
endif()

if (NINTENDO_3DS)
    set(PLATFORM_SDL ON)
    set(PLATFORM_3DS ON)
endif()

if (NINTENDO_WIIU)
    set(PLATFORM_SDL2 ON)
    set(PLATFORM_WIIU ON)
endif()

if (VITA)
    set(PLATFORM_SDL2 ON)
    set(PLATFORM_VITA ON)
    set(SMS_PIXEL_WIDTH 16)
endif()

if (PSP)
    set(PLATFORM_SDL2 ON)
    set(PLATFORM_PSP ON)
    set(SMS_PIXEL_WIDTH 16)
endif()

if (PS2)
    set(PLATFORM_SDL ON)
    set(PLATFORM_PS2 ON)
    set(SMS_PIXEL_WIDTH 16)
endif()

if (DREAMCAST)
    set(PLATFORM_SDL ON)
    set(PLATFORM_DREAMCAST ON)
    set(SMS_PIXEL_WIDTH 16)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "PlayStation")
    set(PLATFORM_PSX ON)
endif()

if (N64)
    set(PLATFORM_N64 ON)
    set(SMS_PIXEL_WIDTH 16)
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
