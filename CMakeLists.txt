cmake_minimum_required(VERSION 3.13.4)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

project(TotalSMS
    VERSION 0.0.1
    DESCRIPTION "WIP SMS emulator!"
    HOMEPAGE_URL "https://github.com/ITotalJustice/TotalSMS"
    LANGUAGES C
)


option(SMS_SINGLE_FILE "include all src in single.c" OFF)
option(SMS_DEBUG "enable debug" OFF)
option(SMS_DEV "enables debug and sanitizers" OFF)
option(SMS_ENABLE_FORCE_INLINE "enables force inline, disabled in debug mode" ON)

option(SMS_EXAMPLE_SDL "" OFF)

option(SMS_TEST_AUDIO "" OFF)
option(SMS_TEST_GFX "" OFF)
option(SMS_TEST_ROMRUNNER "" OFF)
option(SMS_TEST_ALL "build all tests" OFF)


if (SMS_DEV)
    set(SMS_DEBUG ON)
endif()

if (SMS_TEST_ALL)
    set(SMS_TEST_AUDIO ON)
    set(SMS_TEST_GFX ON)
    set(SMS_TEST_ROMRUNNER ON)
endif()


add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)
