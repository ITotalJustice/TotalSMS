cmake_minimum_required(VERSION 3.9)

add_executable(TotalSMS_NDS main.c)
target_link_libraries(TotalSMS_NDS PRIVATE TotalSMS)
nds_create_rom(TotalSMS_NDS)

target_apply_lto_in_release(TotalSMS_NDS)

set_target_properties(TotalSMS_NDS PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
