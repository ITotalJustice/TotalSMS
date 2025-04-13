cmake_minimum_required(VERSION 3.18)

set(USE_ARGS ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/platforms/common binary_dir)

add_executable(TotalSMS_EXE
    main.c
    text_popup.c
    ${COMMON_SOURCES}
)

if (EMSCRIPTEN)
    set_target_properties(TotalSMS_EXE PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    target_link_options(TotalSMS_EXE PRIVATE
        "-s;--shell-file;${CMAKE_CURRENT_SOURCE_DIR}/emscripten.html"
        "-s;EXPORTED_FUNCTIONS=[_main,_malloc,_free]"
        "-lidbfs.js"
        "-sWASM=1"
        "-sALLOW_MEMORY_GROWTH=1"
    )

    option(EMRUN "enable to use emrun (used for debugging)" OFF)

    if (EMRUN)
        target_link_options(TotalSMS_EXE PRIVATE "--emrun")
    endif()
endif()

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.10
    GIT_PROGRESS   TRUE
    FIND_PACKAGE_ARGS 3.2.0 NAMES SDL3
)

FetchContent_MakeAvailable(SDL)

target_link_libraries(TotalSMS_EXE PRIVATE
    TotalSMS
    mgb
    SDL3::SDL3
)

target_include_directories(TotalSMS_EXE PRIVATE
    ${COMMON_INCLUDES}
)

target_add_common_cflags(TotalSMS_EXE PRIVATE)
