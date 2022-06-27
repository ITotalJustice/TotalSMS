cmake_minimum_required(VERSION 3.13.4)

project(zlib LANGUAGES C)

add_library(zlib
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzclose.c
    gzlib.c
    gzread.c
    gzwrite.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    trees.c
    uncompr.c
    zutil.c
)
add_library(ZLIB::ZLIB ALIAS ${PROJECT_NAME})
target_include_directories(zlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
