cmake_minimum_required(VERSION 3.13.4)

add_library(ups ups.c)
target_include_directories(ups PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(ups PRIVATE c_std_99)
