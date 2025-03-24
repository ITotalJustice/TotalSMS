cmake_minimum_required(VERSION 3.18)

add_executable(TotalSMS_Emscripten main.c)

set_target_properties(TotalSMS_Emscripten PROPERTIES
    OUTPUT_NAME "index"
    SUFFIX ".html"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

target_link_options(TotalSMS_Emscripten PRIVATE
    "-s;--shell-file;${CMAKE_CURRENT_SOURCE_DIR}/emscripten.html"
    "-s;EXPORTED_FUNCTIONS=[_main,_malloc,_free]"
    "-lidbfs.js"
    "-sWASM=1"
    "-sALLOW_MEMORY_GROWTH=1"
)

option(EMRUN "enable to use emrun (used for debugging)" OFF)

if (EMRUN)
    target_link_options(TotalSMS_Emscripten PRIVATE "--emrun")
endif()

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(sdl
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.8
    GIT_PROGRESS   TRUE
    FIND_PACKAGE_ARGS NAMES SDL3
)

FetchContent_MakeAvailable(sdl)

target_link_libraries(TotalSMS_Emscripten PRIVATE
    TotalSMS
    mgb
    SDL3::SDL3
)

target_add_common_cflags(TotalSMS_Emscripten PRIVATE)
target_apply_lto_in_release(TotalSMS_Emscripten)
