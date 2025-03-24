cmake_minimum_required(VERSION 3.13.4)

# include(FetchContent)

# FetchContent_Declare(rewind
#     GIT_REPOSITORY https://github.com/ITotalJustice/librewind.git
#     GIT_TAG        v1.2.0
# )

# FetchContent_MakeAvailable(rewind)

add_library(mgb
    mgb.c
    filedialog.c
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
option(STATIC_NFD OFF)

if (VITA)
    set(STATIC_MINIZIP ON)
endif()

if (FORCE_STATIC_LIBS)
    set(STATIC_MINIZIP ON)
    set(STATIC_ZLIB ON)
    set(STATIC_NFD ON)
endif()

add_static_lib(minizip minizip)
find_shared_lib(unofficial-nativefiledialog nativefiledialog)
find_shared_lib(ZLIB zlib)

target_add_common_cflags(mgb PRIVATE)
target_apply_lto_in_release(mgb)

target_link_libraries(mgb PRIVATE
    TotalSMS
    # rewind
    minizip::minizip
    ZLIB::ZLIB
)

if (HAS_NFD OR unofficial-nativefiledialog_FOUND)
    target_compile_definitions(mgb PRIVATE HAS_NFD=1)
    target_link_libraries(mgb PRIVATE unofficial::nativefiledialog::nfd)
endif()
