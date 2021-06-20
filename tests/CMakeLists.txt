cmake_minimum_required(VERSION 3.13.4)


# if (SMS_TEST_SANITY)
#     add_executable(test_sanity test_sanity.c)
#     target_link_libraries(test_sanity LINK_PRIVATE TotalSMS)
# endif()

# if (SMS_TEST_BENCH)
#     add_executable(test_bench test_bench.c)
#     target_link_libraries(test_bench LINK_PRIVATE TotalSMS)
# endif()

if (SMS_TEST_ROMRUNNER)
    add_executable(test_romrunner test_romrunner.c)
    target_link_libraries(test_romrunner LINK_PRIVATE TotalSMS)
endif()

if (SMS_TEST_AUDIO)
    add_executable(test_audio test_audio.c)
    target_link_libraries(test_audio LINK_PRIVATE TotalSMS)

    find_package(SDL2 REQUIRED)
    target_include_directories(test_audio PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(test_audio PRIVATE ${SDL2_LIBRARIES})
endif()
