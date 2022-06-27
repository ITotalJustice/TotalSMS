cmake_minimum_required(VERSION 3.13.4)

add_library(gameshark gameshark.c)
target_include_directories(gameshark PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(gameshark PRIVATE c_std_99)
