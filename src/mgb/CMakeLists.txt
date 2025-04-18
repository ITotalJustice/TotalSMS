cmake_minimum_required(VERSION 3.13.4)

include(FetchContent)

FetchContent_Declare(patch
    GIT_REPOSITORY https://github.com/ITotalJustice/libpatch.git
    GIT_TAG        v1.1.0
)

FetchContent_MakeAvailable(patch)

add_library(mgb
    mgb.c
    romloader.c
    util.c
    directory.c
    png/png.c

    ifile/ifile.c
    ifile/zip/zip.c
    ifile/mem/mem.c
    ifile/cfile/cfile.c
)

target_include_directories(mgb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_add_common_cflags(mgb PRIVATE)

set(ZLIB_FOUND FALSE)
set(MINIZIP_FOUND FALSE)

find_package(ZLIB QUIET)

if (NOT ZLIB_FOUND OR NOT DEFINED ZLIB_VERSION)
    set(ZLIB_VERSION 1.3.1)

    FetchContent_Declare(zlib
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        v${ZLIB_VERSION}
        GIT_PROGRESS   TRUE
        SOURCE_SUBDIR NONE
    )

    FetchContent_MakeAvailable(zlib)

    add_library(zlib
        ${zlib_SOURCE_DIR}/adler32.c
        ${zlib_SOURCE_DIR}/compress.c
        ${zlib_SOURCE_DIR}/crc32.c
        ${zlib_SOURCE_DIR}/deflate.c
        # ${zlib_SOURCE_DIR}/gzclose.c
        # ${zlib_SOURCE_DIR}/gzlib.c
        # ${zlib_SOURCE_DIR}/gzread.c
        # ${zlib_SOURCE_DIR}/gzwrite.c
        ${zlib_SOURCE_DIR}/infback.c
        ${zlib_SOURCE_DIR}/inffast.c
        ${zlib_SOURCE_DIR}/inflate.c
        ${zlib_SOURCE_DIR}/inftrees.c
        ${zlib_SOURCE_DIR}/trees.c
        ${zlib_SOURCE_DIR}/uncompr.c
        ${zlib_SOURCE_DIR}/zutil.c
    )

    add_library(ZLIB::ZLIB ALIAS zlib)
    target_include_directories(zlib INTERFACE ${zlib_SOURCE_DIR})
    message(STATUS "found zlib")
endif()

# find_package() for minizip is broken on fedora
# not sure other distros have .cmake for minizip either
# so using the old fashioned find_() has to be used
find_library(minizip_lib minizip)
find_path(minizip_inc minizip)

if (minizip_lib AND minizip_inc)
    target_link_libraries(mgb PRIVATE ${minizip_lib})
    target_include_directories(mgb PRIVATE ${minizip_inc})

    # this has to be linked after minizip!
    target_link_libraries(mgb PRIVATE ZLIB::ZLIB)

    set(MINIZIP_FOUND TRUE)
    message(STATUS "using system minizip")
endif()

# failed to find minizip installed, fetch and build manually
if (NOT MINIZIP_FOUND)
    message(STATUS "populating minizip")
    # fetch the same version as zlib :)
    set(MINIZIP_VERSION ${ZLIB_VERSION})
    # work around bug where v1.3 is defined as v1.3.0
    if (MINIZIP_VERSION EQUAL 1.3.0)
        set(MINIZIP_VERSION 1.3)
    endif()

    FetchContent_Declare(minizip
        GIT_REPOSITORY https://github.com/madler/zlib.git
        GIT_TAG        v${MINIZIP_VERSION}
        GIT_PROGRESS   TRUE
        SOURCE_SUBDIR NONE
    )

    FetchContent_MakeAvailable(minizip)

    add_library(minizip
        ${minizip_SOURCE_DIR}/contrib/minizip/ioapi.c
        ${minizip_SOURCE_DIR}/contrib/minizip/zip.c
        ${minizip_SOURCE_DIR}/contrib/minizip/unzip.c
    )

    target_include_directories(minizip PRIVATE ${minizip_SOURCE_DIR}/contrib/minizip/)
    # need the paths to be minizip/unzip.h
    target_include_directories(minizip PUBLIC ${minizip_SOURCE_DIR}/contrib/)

    target_link_libraries(minizip PRIVATE ZLIB::ZLIB)
    target_link_libraries(mgb PRIVATE minizip)

    message(STATUS "using github minizip")
endif()

target_link_libraries(mgb PRIVATE
    SMS_Core
    patch
    ZLIB::ZLIB
)
