cmake_minimum_required(VERSION 3.13.4)


# todo: support multiple libretro console lib outputs
add_library(totalsms_libretro
    main_libretro.c
)

set_target_properties(totalsms_libretro PROPERTIES PREFIX "")
target_link_libraries(totalsms_libretro LINK_PUBLIC TotalSMS)

include(ExternalProject)
include(FetchContent)

if (SWITCH)
    add_custom_target(retro_libnx.a ALL
        COMMAND ar -x ${CMAKE_BINARY_DIR}/platforms/libretro/totalsms_libretro.a
        COMMAND ar -x ${CMAKE_BINARY_DIR}/src/libTotalSMS.a
        COMMAND ar -qc retro_libnx.a *.obj
        DEPENDS totalsms_libretro
    )

    # ExternalProject_Add(fetchRA
    #     GIT_REPOSITORY https://github.com/libretro/RetroArch.git
    #     GIT_SHALLOW ON
    #     GIT_PROGRESS ON

    #     CONFIGURE_COMMAND ""
    #     # COMMAND ${CMAKE_COMMAND} --verbose -E copy retro_libnx.a <BINARY_DIR>/retro_libnx.a
    #     # WORKING_DIRECTORY <SOURCE_DIR>/
    #     # BUILD_COMMAND make -f <SOURCE_DIR>/Makefile.libnx

    #     DEPENDS retro_libnx.a
    # )

    FetchContent_Declare(fetchRA
        GIT_REPOSITORY https://github.com/libretro/RetroArch.git
        GIT_SHALLOW ON
        GIT_PROGRESS ON

        # CONFIGURE_COMMAND ""
        # COMMAND ${CMAKE_COMMAND} --verbose -E copy retro_libnx.a <BINARY_DIR>/retro_libnx.a
        # WORKING_DIRECTORY <SOURCE_DIR>/
        # BUILD_COMMAND make -f <SOURCE_DIR>/Makefile.libnx
    )

    FetchContent_MakeAvailable(fetchRA)

    add_custom_target(idk ALL
        COMMAND ${CMAKE_COMMAND} -E copy retro_libnx.a ${fetchRA_SOURCE_DIR}/retro_libnx.a
        COMMAND chdir ${fetchRA_SOURCE_DIR}
        COMMAND make -f Makefile.libnx
        DEPENDS retro_libnx.a
    )
endif()

# set(CMAKE_TLS_VERIFY true)

# FetchContent_Declare(retroarch
#     URL "https://github.com/libretro/RetroArch/archive/refs/heads/master.zip"
#     DOWNLOAD_NO_EXTRACT false
# )

# FetchContent_Declare(retroarch
#     GIT_REPOSITORY "https://github.com/libretro/RetroArch.git"
#     GIT_TAG master
#     GIT_SHALLOW TRUE # TODO get only last version
#     GIT_PROGRESS TRUE # show progress of download
# )

# FetchContent_MakeAvailable(retroarch)

# add_custom_target(libvz ALL
#     /usr/bin/make -f "${CMAKE_CURRENT_SOURCE_DIR}/retroarch-src/Makefile.libnx"
# )
# add_custom_command(
#     OUTPUT idk
#     COMMAND make -f Makefile.libnx
#     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/retroarch-src
# )
