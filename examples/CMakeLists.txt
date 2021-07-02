cmake_minimum_required(VERSION 3.13.4)


if (SMS_EXAMPLE_SDL OR ${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
        add_executable(index example_sdl.c)
        target_link_libraries(index LINK_PRIVATE TotalSMS)

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=[png] -s USE_ZLIB")
        set(CMAKE_EXECUTABLE_SUFFIX .html)
    
        set_target_properties(index PROPERTIES LINK_FLAGS
            "-s USE_SDL=2 \
            -s USE_SDL_IMAGE=2 \
            -s SDL2_IMAGE_FORMATS=[png] \
            -s USE_ZLIB \
            -s WASM=1 \
            -lidbfs.js \
            -s EXPORTED_FUNCTIONS=[_main,_malloc,_free] \
            -s --shell-file ${PROJECT_SOURCE_DIR}/examples/example_emscripten.html \
            --preload-file res \
            -flto \
            "
        )
    else()
        add_executable(example_sdl example_sdl.c)
        target_link_libraries(example_sdl LINK_PRIVATE TotalSMS)

        find_package(SDL2 REQUIRED)
        find_package(SDL2_image REQUIRED)
        find_package(ZLIB REQUIRED)
        
        target_include_directories(example_sdl PRIVATE
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
            ${ZLIB_INCLUDE_DIRS}
        )

        target_link_libraries(example_sdl PRIVATE
            ${SDL2_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            ${ZLIB_LIBRARIES}
        )
    endif()

    configure_file("${CMAKE_SOURCE_DIR}/res/controller_mapping/gamecontrollerdb.txt" # src
      "${CMAKE_BINARY_DIR}/examples/res/controller_mapping/gamecontrollerdb.txt" # dst
      COPYONLY
    )

    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/a.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/a.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/b.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/b.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/up.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/up.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/down.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/down.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/left.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/left.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/right.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/right.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/pause.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/pause.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/menu.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/menu.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/title.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/title.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/save.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/save.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/load.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/load.png" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/sprites/back.png" "${CMAKE_BINARY_DIR}/examples/res/sprites/back.png" COPYONLY)
endif()
