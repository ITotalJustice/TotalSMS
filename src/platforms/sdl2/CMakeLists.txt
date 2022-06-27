cmake_minimum_required(VERSION 3.18.0)

# NOTE: i don't remember half the stuff in this cmake file, i should
# so it may not work on all platforms anymore.
# todo: rewrite it all.

list(APPEND src main.c)

set(myname "TotalSMS_SDL2")
set(sdl2_static FALSE)

if (ANDROID)
    set(myname "main")
    add_library(${myname} SHARED ${src})
# elseif (WIN32)
#     # adds icon to exe
#     list(APPEND src "${CMAKE_SOURCE_DIR}/res/windows/icon.rc")
#     # WIN32 builds as gui application, so no console!
#     # SEE: https://cmake.org/cmake/help/latest/command/add_executable.html
#     add_executable(${myname} WIN32 ${src})
#     find_package(SDL2 CONFIG REQUIRED)
elseif (PSP)
    add_executable(${myname} ${src})
    # add_subdirectory(SDL) # ???
    set(sdl2_static TRUE)
else()
    add_executable(${myname} ${src})
    find_package(SDL2 CONFIG REQUIRED)
endif()

target_link_libraries(${myname} PRIVATE
    TotalSMS
    SDL2::SDL2main
    mgb
)

if (NINTENDO_SWITCH)
    set_target_properties(${myname} PROPERTIES LINKER_LANGUAGE CXX)

    nx_generate_nacp(${myname}.nacp
        NAME ${myname}
        AUTHOR TotalJustice
        VERSION 0.0.1
    )

    nx_create_nro(${myname}
        # ICON ${CMAKE_SOURCE_DIR}/res/icons/icon.jpg
        NACP ${myname}.nacp
        # ROMFS ${CMAKE_CURRENT_BINARY_DIR}/romfs
    )
endif()

if (WIIU)
    set(sdl2_static TRUE)
    wut_create_rpx(${myname})
endif()

if (VITA)
    set(sdl2_static TRUE)
    include("${VITASDK}/share/vita.cmake" REQUIRED)

    set(VITA_APP_NAME ${myname})
    set(VITA_TITLEID  "VSDK00017")
    set(VITA_VERSION  "01.00")

    target_link_libraries(${myname} PRIVATE
        SceDisplay_stub
        SceCtrl_stub
        SceAudio_stub
        SceSysmodule_stub
        SceGxm_stub
        SceCommonDialog_stub
        SceAppMgr_stub
        SceTouch_stub
        SceHid_stub
        SceMotion_stub
        m
    )

    vita_create_self(${myname}.self ${myname})
    vita_create_vpk(${myname}.vpk ${VITA_TITLEID} ${myname}.self
        VERSION ${VITA_VERSION}
        NAME ${VITA_APP_NAME}
        # FILE sce_sys/icon0.png sce_sys/icon0.png
        # FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
        # FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
        # FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
    )
endif()

if (PSP)
    set(sdl2_static TRUE)
    target_link_libraries(${myname} PRIVATE
        GL
        pspvram
        pspaudio
        pspvfpu
        pspdisplay
        pspgu
        pspge
        pspdebug
        psphprm
        pspctrl
    )

    create_pbp_file(TARGET ${myname} # CMake executable target
        TITLE "${CMAKE_PROJECT_NAME}" # displayed in game selection
        # ICON_PATH
        # BACKGROUND_PATH
        # PREVIEW_PATH
    )
endif()

if (VCPKG_TOOLCHAIN)
    message("building against vcpkg!!!")

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set(sdl2_static TRUE)
    endif()
endif()

if (sdl2_static)
    target_link_libraries(${myname} PRIVATE SDL2::SDL2-static)
else()
    target_link_libraries(${myname} PRIVATE SDL2::SDL2)
endif()

target_add_common_cflags(${myname} PRIVATE)

set_target_properties(${myname} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
