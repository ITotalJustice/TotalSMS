cmake_minimum_required(VERSION 3.13.4)

add_library(codebreaker codebreaker.c)
target_include_directories(codebreaker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(codebreaker PRIVATE c_std_99)
