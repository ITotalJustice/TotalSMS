cmake_minimum_required(VERSION 3.13.4)

add_library(gamegenie gamegenie.c)
target_include_directories(gamegenie PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(gamegenie PRIVATE c_std_99)
