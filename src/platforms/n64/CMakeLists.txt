cmake_minimum_required(VERSION 3.18.0)

add_executable(TotalSMS_N64 main.c)
target_link_libraries(TotalSMS_N64 PRIVATE TotalSMS)

n64_create_z64(TotalSMS_N64
    NAME "TotalSMS_N64"
    # HEADER
    # SIZE 4M
)
