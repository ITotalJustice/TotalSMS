cmake_minimum_required(VERSION 3.13.4)

# include(FetchContent)

# FetchContent_Declare(rewind
#     GIT_REPOSITORY https://github.com/ITotalJustice/librewind.git
#     GIT_TAG        v1.2.0
# )

# FetchContent_MakeAvailable(rewind)

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
    # rewind.c
    compressors.c
    directory.c
    png/png.c

    ifile/ifile.c
    ifile/zip/zip.c
    ifile/mem/mem.c
    ifile/cfile/cfile.c
)

target_include_directories(mgb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

option(FORCE_STATIC_LIBS OFF)
option(STATIC_MINIZIP OFF)
option(STATIC_ZLIB OFF)

if (VITA)
    set(STATIC_MINIZIP ON)
endif()

if (FORCE_STATIC_LIBS)
    set(STATIC_MINIZIP ON)
    set(STATIC_ZLIB ON)
endif()

add_static_lib(minizip minizip)
find_shared_lib(ZLIB zlib)

target_add_common_cflags(mgb PRIVATE)

target_link_libraries(mgb PRIVATE
    TotalSMS
    # rewind
    minizip::minizip
    ZLIB::ZLIB
    patch
)
