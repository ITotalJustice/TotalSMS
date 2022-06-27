cmake_minimum_required(VERSION 3.13.4)

add_library(ips ips.c)
target_include_directories(ips PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(ips PRIVATE c_std_99)
