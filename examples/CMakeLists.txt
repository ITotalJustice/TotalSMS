cmake_minimum_required(VERSION 3.13.4)


if (SMS_EXAMPLE_SDL OR ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
        add_executable(index example_sdl.c)
        target_link_libraries(index LINK_PRIVATE TotalSMS)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2")
        set(CMAKE_EXECUTABLE_SUFFIX .html)
    
        set_target_properties(index PROPERTIES LINK_FLAGS
            "-s USE_SDL=2 \
            -s WASM=1 \
            -lidbfs.js \
            -s EXPORTED_FUNCTIONS=[_main,_malloc,_free] \
            -s --shell-file ${PROJECT_SOURCE_DIR}/examples/example_emscripten.html \
            --preload-file res \
            "
        )
    else()
        add_executable(example_sdl example_sdl.c)
        target_link_libraries(example_sdl LINK_PRIVATE TotalSMS)

        find_package(SDL2 REQUIRED)
        target_include_directories(example_sdl PRIVATE ${SDL2_INCLUDE_DIRS})
        target_link_libraries(example_sdl PRIVATE ${SDL2_LIBRARIES})
    endif()

    configure_file("${CMAKE_SOURCE_DIR}/res/controller_mapping/gamecontrollerdb.txt" # src
      "${CMAKE_BINARY_DIR}/examples/res/controller_mapping/gamecontrollerdb.txt" # dst
      COPYONLY
    )

    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/a.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/a.bmp" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/b.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/b.bmp" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/up.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/up.bmp" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/down.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/down.bmp" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/left.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/left.bmp" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/right.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/right.bmp" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/fullscreen.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/fullscreen.bmp" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/touch_buttons/file.bmp" "${CMAKE_BINARY_DIR}/examples/res/touch_buttons/file.bmp" COPYONLY)
endif()
