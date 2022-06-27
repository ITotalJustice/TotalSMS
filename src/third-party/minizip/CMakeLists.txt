cmake_minimum_required(VERSION 3.13.4)

project(minizip LANGUAGES C)

add_library(minizip
    ioapi_mem.c
    ioapi.c
    unzip.c
    zip.c
)
add_library(minizip::minizip ALIAS ${PROJECT_NAME})

target_include_directories(minizip INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(minizip PRIVATE NOCRYPT=1)
target_compile_definitions(minizip PRIVATE NOUNCRYPT=1)

find_shared_lib(ZLIB zlib)
target_link_libraries(minizip LINK_PRIVATE ZLIB::ZLIB)
