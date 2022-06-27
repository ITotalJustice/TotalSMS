# todo: check env vars for if the user set a different path
set(KOS_BASE "/opt/toolchains/dc/kos")
set(KOS_PORTS "${KOS_BASE}/../kos-ports")

find_program(MKISOFS mkisofs)
find_program(SCRAMBLE scramble ${KOS_BASE}/utils/scramble)

set(KOS_CC_BASE "/opt/toolchains/dc/sh-elf")
set(KOS_CC_PREFIX "sh-elf")

set(KOS_ARCH "dreamcast")
set(KOS_ARCH_DIR "${KOS_BASE}/kernel/arch/${KOS_ARCH}")

set(KOS_SUBARCH "pristine")

# Our includes
set(KOS_INC_PATHS "-I${KOS_BASE}/include -I${KOS_BASE}/kernel/arch/${KOS_ARCH}/include -I${KOS_BASE}/addons/include -I${KOS_PORTS}/include")

# "System" libraries
set(KOS_LIB_PATHS "-L${KOS_BASE}/lib/${KOS_ARCH} -L${KOS_BASE}/addons/lib/${KOS_ARCH} -L${KOS_PORTS}/lib")

set(KOS_LIBS "-Wl,--start-group -lkallisti -lc -lgcc -Wl,--end-group")

set(KOS_CFLAGS "${KOS_INC_PATHS} -D_arch_${KOS_ARCH} -D_arch_sub_${KOS_SUBARCH} -Wall")
set(KOS_CPPFLAGS "${KOS_INC_PATHS_CPP} -fno-operator-names -fno-rtti -fno-exceptions")

set(KOS_LD_SCRIPT "-T${KOS_BASE}/utils/ldscripts/shlelf.xc")
set(KOS_LINKER_FLAGS "${KOS_LD_SCRIPT} -nodefaultlibs ${KOS_LIB_PATHS}")

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_SYSTEM_PROCESSOR sh4)

SET(CMAKE_C_COMPILER ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-gcc)
SET(CMAKE_CXX_COMPILER ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-g++)
set(CMAKE_LINKER ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-ld)
set(CMAKE_AR ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-ar)
set(CMAKE_OBJCOPY ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-objcopy)
set(CMAKE_STRIP ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-strip)
set(CMAKE_ADDR2LINE ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-addr2line)
set(CMAKE_RANLIB ${KOS_CC_BASE}/bin/${KOS_CC_PREFIX}-ranlib)

SET(CMAKE_C_FLAGS_INIT "${KOS_CFLAGS} -Ofast -ml -m4-single-only -fno-builtin -ffunction-sections -fdata-sections")
SET(CMAKE_CXX_FLAGS_INIT "${KOS_CPPFLAGS} -Ofast -ml -m4-single-only -fno-builtin -ffunction-sections -fdata-sections")
SET(CMAKE_EXE_LINKER_FLAGS_INIT "${KOS_LIB_PATHS} ${KOS_LINKER_FLAGS} -ml -m4-single-only -fno-builtin -Wl,-Ttext=0x8c010000 -Wl,--gc-sections")

SET(CMAKE_FIND_ROOT_PATH ${KOS_BASE} ${KOS_BASE}/include ${KOS_PORTS})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

SET(CMAKE_EXECUTABLE_SUFFIX ".elf") # is this needed?
SET(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
SET(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

link_libraries(${KOS_LIBS})
add_definitions("-D__DREAMCAST__ -DDREAMCAST")
SET(PLATFORM_DREAMCAST TRUE)
SET(DREAMCAST TRUE)

function(dc_create_bin target)
    add_custom_command(OUTPUT ${target}.bin
        COMMAND ${CMAKE_OBJCOPY} -R .stack -O binary ${target}.elf ${target}.bin
        DEPENDS ${target}
    )

    add_custom_target(
		"${target}_bin" ALL
		DEPENDS "${target}.bin"
	)
endfunction()

function(dc_scramble target)
    dc_create_bin(${target})

    add_custom_command(OUTPUT 1ST_READ.BIN
        COMMAND ${SCRAMBLE} ${target}.bin 1ST_READ.BIN
        DEPENDS ${target}
    )

    add_custom_target(
		"1ST_READ_BIN" ALL
		DEPENDS "1ST_READ.BIN"
	)
endfunction()
