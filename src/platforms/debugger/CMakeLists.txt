cmake_minimum_required(VERSION 3.20.0)

project(TotalSMS_Debugger
    LANGUAGES C CXX
)

include(FetchContent)

Set(FETCHCONTENT_QUIET FALSE)

FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        697ce2d67ba7c684747ab1cce2b7d897e3e698c0
    GIT_PROGRESS   TRUE
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_Declare(imgui_club
    GIT_REPOSITORY https://github.com/ocornut/imgui_club.git
    GIT_TAG        d4cd9896e15a03e92702a578586c3f91bbde01e8
    GIT_PROGRESS   TRUE
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
)

FetchContent_MakeAvailable(imgui)
FetchContent_MakeAvailable(imgui_club)

add_library(imgui
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

target_include_directories(imgui
    PUBLIC
        ${imgui_SOURCE_DIR}

    INTERFACE
        ${imgui_SOURCE_DIR}/backends
)

add_library(imgui_club INTERFACE)
target_include_directories(imgui INTERFACE ${imgui_club_SOURCE_DIR}/imgui_memory_editor)

add_executable(TotalSMS_Debugger
    main.cpp
    imgui_backend.cpp
    io_debugger.cpp

    mapper_debugger.cpp
    psg_debugger.cpp
    vdp_debugger.cpp
    z80_debugger.cpp

    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer.cpp
)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(TotalSMS_Debugger PRIVATE
    TotalSMS
    mgb
    imgui
    imgui_club
    SDL2::SDL2
)

set_target_properties(TotalSMS_Debugger PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CXX_STANDARD 20
)
