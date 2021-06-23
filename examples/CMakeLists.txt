cmake_minimum_required(VERSION 3.13.4)


if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    add_executable(index example_emscripten.c)
    target_link_libraries(index PRIVATE TotalSMS)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2")
    
    set_target_properties(index PROPERTIES LINK_FLAGS
        "-s USE_SDL=2 \
        -s WASM=1 \
        -lidbfs.js \
        -s EXPORTED_FUNCTIONS=[_main,_malloc,_free] \
        -s --shell-file ${PROJECT_SOURCE_DIR}/examples/example_emscripten.html \
        "
    )

    configure_file("${CMAKE_SOURCE_DIR}/res/controller_mapping/gamecontrollerdb.txt" # src
      "${CMAKE_BINARY_DIR}/res/gamecontrollerdb.txt" # dst
      COPYONLY
    )

    set(CMAKE_EXECUTABLE_SUFFIX .html)
endif()

if (SMS_EXAMPLE_SDL)
    add_executable(example_sdl example_sdl.c)
    target_link_libraries(example_sdl LINK_PRIVATE TotalSMS)

    find_package(SDL2 REQUIRED)
    target_include_directories(example_sdl PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(example_sdl PRIVATE ${SDL2_LIBRARIES})

    configure_file("${CMAKE_SOURCE_DIR}/res/controller_mapping/gamecontrollerdb.txt" # src
      "${CMAKE_BINARY_DIR}/res/gamecontrollerdb.txt" # dst
      COPYONLY
    )
endif()
