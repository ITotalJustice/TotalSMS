cmake_minimum_required(VERSION 3.18)

find_package(SDL2 CONFIG REQUIRED)

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
)

option(EMRUN "enable to use emrun (used for debugging)" OFF)

if (EMRUN)
    target_link_options(TotalSMS_Emscripten PRIVATE "--emrun")
endif()

target_link_libraries(TotalSMS_Emscripten PRIVATE
    TotalSMS
    SDL2::SDL2
    SDL2::SDL2main
    mgb
)

target_add_common_cflags(TotalSMS_Emscripten PRIVATE)
target_apply_lto_in_release(TotalSMS_Emscripten)
