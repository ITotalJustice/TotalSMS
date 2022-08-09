cmake_minimum_required(VERSION 3.18.0)

project(TotalSMS_N64
    LANGUAGES C
    VERSION 0.0.1
)

add_static_lib(minizip minizip)
find_shared_lib(ZLIB zlib)

add_executable(TotalSMS_N64 main.c)
target_link_libraries(TotalSMS_N64 PRIVATE TotalSMS minizip::minizip ZLIB::ZLIB)

target_add_common_cflags(TotalSMS_N64 PRIVATE)
target_apply_lto_in_release(TotalSMS_N64)

n64_create_dfs(
    ${CMAKE_CURRENT_SOURCE_DIR}/roms # src
    roms.dfs # output
)

n64_create_z64(TotalSMS_N64
    NAME "TotalSMS_N64"
    DFS "roms.dfs"
)
