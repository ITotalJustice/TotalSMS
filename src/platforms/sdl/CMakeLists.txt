cmake_minimum_required(VERSION 3.18.0)


set(output "TotalSMS_SDL")

add_executable(${output} main.c)

find_package(SDL REQUIRED)
target_include_directories(${output} PRIVATE ${SDL_INCLUDE_DIRS})
target_link_libraries(${output} PRIVATE ${SDL_LIBRARIES})

target_link_libraries(${output} PRIVATE TotalSMS mgb)

if (NINTENDO_3DS)
    target_link_libraries(${output} PRIVATE citro2d citro3d)
    ctr_create_3dsx(${output})
endif()

if (PS2)
    # set(CMAKE_EXECUTABLE_SUFFIX .elf)
    # target_include_directories(${output} PRIVATE ${PS2SDK}/ports/include/SDL)
    # target_link_libraries(${output} PRIVATE sdl)
endif()

if (DREAMCAST)
    set(CMAKE_EXECUTABLE_SUFFIX .elf)
endif()

if (GAMECUBE)
    target_link_libraries(${output} PRIVATE aesnd)
    target_link_libraries(${output} PRIVATE fat)
    ogc_create_dol(${output})
endif()

target_add_common_cflags(${output} PRIVATE)
target_apply_lto_in_release(${output})

set_target_properties(${output} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
