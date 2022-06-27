cmake_minimum_required(VERSION 3.18.0)

set(output "TotalSMS_GC")

add_executable(${output} main.c)

find_package(SDL REQUIRED)
target_include_directories(${output} PRIVATE ${SDL_INCLUDE_DIRS})
target_link_libraries(${output} PRIVATE ${SDL_LIBRARIES})
target_link_libraries(${output} PRIVATE TotalSMS mgb)
target_link_libraries(${output} PRIVATE aesnd)
target_link_libraries(${output} PRIVATE fat)

ogc_create_dol(${output})

target_add_common_cflags(${output} PRIVATE)
target_apply_lto_in_release(${output})

set_target_properties(${output} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
