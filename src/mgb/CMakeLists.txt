cmake_minimum_required(VERSION 3.13.4)

# disable for now as they're unused
set(IPS OFF)
set(UPS OFF)
set(GAMESHARK OFF)
set(CODEBREAKER OFF)
set(GAMEGENIE OFF)

add_subdirectory(cheats)
add_subdirectory(patch)

add_library(mgb
    mgb.c
    filedialog.c
    romloader.c
    util.c
    rewind.c
    compressors.c
    directory.c

    ifile/ifile.c
    ifile/zip/zip.c
    ifile/mem/mem.c
    ifile/cfile/cfile.c
    ifile/gzip/gzip.c
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
# find_shared_lib(zstd zstd)
# find_shared_lib(lz4 zstd)

# find_package(zstd CONFIG REQUIRED)
# find_package(lz4 CONFIG REQUIRED)

target_add_common_cflags(mgb PRIVATE)
target_apply_lto_in_release(mgb)

target_link_libraries(mgb PRIVATE
    TotalSMS
    minizip::minizip
    ZLIB::ZLIB
)

if (HAS_NFD OR unofficial-nativefiledialog_FOUND)
    target_compile_definitions(mgb PRIVATE HAS_NFD=1)
    target_link_libraries(mgb PRIVATE unofficial::nativefiledialog::nfd)
endif()
