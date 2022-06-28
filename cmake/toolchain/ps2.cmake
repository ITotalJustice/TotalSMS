set(PS2SDK $ENV{PS2SDK})
set(PS2DEV $ENV{PS2DEV})

set(BUILD_EE ON)

if (BUILD_EE)
    set(PREFIX mips64r5900el-ps2-elf-)
    set(LINKFILE ${PS2SDK}/ee/startup/linkfile)
else()
    set(PREFIX mipsel-ps2-irx-)
    # todo: link file
endif()

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_SYSTEM_PROCESSOR mips)
SET(CMAKE_C_COMPILER ${PREFIX}gcc)
SET(CMAKE_CXX_COMPILER ${PREFIX}g++)
set(CMAKE_LINKER ${PREFIX}ld)
set(CMAKE_AR ${PREFIX}ar)
set(CMAKE_OBJCOPY ${PREFIX}objcopy)
set(CMAKE_STRIP ${PREFIX}strip)
set(CMAKE_ADDR2LINE ${PREFIX}addr2line)
set(CMAKE_RANLIB ${PREFIX}ranlib)
SET(CMAKE_C_FLAGS_INIT "-I${PS2SDK}/ee/include -I${PS2SDK}/common/include -O2 -march=r5900 -mtune=r5900")
SET(CMAKE_CXX_FLAGS_INIT "-I${PS2SDK}/ee/include -I${PS2SDK}/common/include -O2 -march=r5900 -mtune=r5900")
SET(CMAKE_EXE_LINKER_FLAGS_INIT "-L${PS2DEV}/lib -L${PS2DEV}/psp/lib -L${PS2SDK}/ee/lib -L${PS2DEV}/psp/sdk/lib")
SET(CMAKE_TARGET_INSTALL_PREFIX ${PS2SDK}/ports)
SET(CMAKE_INSTALL_PREFIX ${PS2SDK}/ports)
SET(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
SET(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

SET(CMAKE_FIND_ROOT_PATH ${PS2DEV} ${PS2DEV}/ee ${PS2DEV}/ee/ee ${PS2SDK} ${PS2SDK}/ports)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_link_options(-T${LINKFILE})
link_libraries("-Wl,-zmax-page-size=128")

add_definitions("-D__PS2__ -DPS2 -D_EE -G0")
SET(PLATFORM_PS2 TRUE)
SET(PS2 TRUE)