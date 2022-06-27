cmake_minimum_required(VERSION 3.13.4)

project(stb LANGUAGES C)

add_library(stb_rect_pack
    stb_rect_pack.c
)

add_library(stb_truetype
    stb_truetype.c
)

add_library(stb_image
    stb_image.c
)

add_library(STB::TRUETYPE ALIAS stb_rect_pack)
add_library(STB::RECT_PACK ALIAS stb_truetype)
add_library(STB::IMAGE ALIAS stb_image)

if (NOT WIN32)
    target_link_libraries(stb_truetype PRIVATE m)
endif()

target_include_directories(stb_rect_pack INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stb_truetype INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(stb_image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(stb_image PRIVATE STBI_NO_BMP)
target_compile_definitions(stb_image PRIVATE STBI_NO_PSD)
target_compile_definitions(stb_image PRIVATE STBI_NO_TGA)
target_compile_definitions(stb_image PRIVATE STBI_NO_GIF)
target_compile_definitions(stb_image PRIVATE STBI_NO_HDR)
target_compile_definitions(stb_image PRIVATE STBI_NO_PIC)
target_compile_definitions(stb_image PRIVATE STBI_NO_PNM)
# target_compile_definitions(stb_image PRIVATE STBI_NO_ZLIB)
# target_compile_definitions(stb_image PRIVATE STBI_NO_PNG)
# target_compile_definitions(stb_image PRIVATE STBI_NO_JPEG)
target_compile_definitions(stb_image PRIVATE STBI_NO_STDIO)
