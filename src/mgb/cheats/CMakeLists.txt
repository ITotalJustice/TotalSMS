cmake_minimum_required(VERSION 3.13.4)

if (GAMESHARK)
    message(STATUS "GameShark support enabled!")
    add_subdirectory(gameshark)
endif()

if (CODEBREAKER)
    message(STATUS "CodeBreaker support enabled!")
    add_subdirectory(codebreaker)
endif()

if (GAMEGENIE)
    message(STATUS "GameGenie support enabled!")
    add_subdirectory(gamegenie)
endif()
