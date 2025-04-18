cmake_minimum_required(VERSION 3.18)

set(USE_ARGS ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/src/platforms/common binary_dir)

add_executable(TotalSMS
    main.c
    text_popup.c
    rewind_bar.c
    rewind.c
    ${COMMON_SOURCES}
)

if (EMSCRIPTEN)
    set_target_properties(TotalSMS PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    target_link_options(TotalSMS PRIVATE
        "-s;--shell-file;${CMAKE_CURRENT_SOURCE_DIR}/emscripten.html"
        "-s;EXPORTED_FUNCTIONS=[_main,_malloc,_free]"
        "-lidbfs.js"
        "-sWASM=1"
        "-sUSE_SDL=3"
        "-sALLOW_MEMORY_GROWTH=1"
    )

    option(EMRUN "enable to use emrun (used for debugging)" OFF)

    if (EMRUN)
        target_link_options(TotalSMS PRIVATE "--emrun")
    endif()
endif()

include(FetchContent)
Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(SDL
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.10
    GIT_PROGRESS   TRUE
    FIND_PACKAGE_ARGS NAMES SDL3
)

set(LZ4_BUILD_CLI OFF)

FetchContent_Declare(lz4
    GIT_REPOSITORY https://github.com/lz4/lz4.git
    GIT_TAG        v1.10.0
    GIT_PROGRESS   TRUE
    SOURCE_SUBDIR  build/cmake
    FIND_PACKAGE_ARGS NAMES lz4
)

FetchContent_MakeAvailable(SDL lz4)

target_link_libraries(TotalSMS PRIVATE
    SMS_Core
    mgb
    SDL3::SDL3
    lz4
)

target_include_directories(TotalSMS PRIVATE
    ${COMMON_INCLUDES}
)

target_add_common_cflags(TotalSMS PRIVATE)

# CPack
# cpack --config CPackConfig.cmake
# cpack --config CPackSourceConfig.cmake
# cpack -G RPM
install(TARGETS TotalSMS RUNTIME DESTINATION bin)
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_CONTACT "TotalJustice")
set(CPACK_STRIP_FILES TRUE)

# todo
# set(CPACK_PACKAGE_ICON)

# RPM stuff
set(CPACK_RPM_PACKAGE_LICENSE MIT)

# DEB stuff
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS TRUE)

set(CPACK_SOURCE_IGNORE_FILES
    .git/
    .\*~
    \.DS_Store

    # copied from .gitignore
    build*/
    web/
    roms/
    *.zip
    *.raw
    ignoreme/
    compile_commands.json
    .cache
    libnx
    n64-emu
    rom*.h
    out.h
    perf.*
    utils/
    old/
    imgui.ini
    .vscode
    CMakeUserPresets.json

    sdl3
    src/platforms/blip
)

include(CPack)
