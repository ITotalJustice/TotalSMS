cmake_minimum_required(VERSION 3.13.4)


if (TEST_PERF)
    add_executable(test_perf test_perf.c)
    target_link_libraries(test_perf PRIVATE TotalSMS)
endif()

if (TEST_ROMRUNNER)
    add_executable(test_romrunner test_romrunner.c)
    target_link_libraries(test_romrunner PRIVATE TotalSMS)
endif()

if (TEST_AUDIO)
    add_executable(test_audio test_audio.c)
    target_link_libraries(test_audio PRIVATE TotalSMS)

    find_package(SDL2 REQUIRED)
    target_include_directories(test_audio PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(test_audio PRIVATE ${SDL2_LIBRARIES})
endif()

if (TEST_GFX)
    add_executable(test_gfx test_gfx.c)
    target_link_libraries(test_gfx PRIVATE TotalSMS)

    find_package(SDL2 REQUIRED)
    target_include_directories(test_gfx PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(test_gfx PRIVATE ${SDL2_LIBRARIES})
endif()
