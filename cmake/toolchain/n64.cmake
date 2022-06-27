# to set your own path, use -DLIBDRAGON=/path/to/libragon
if (NOT LIBDRAGON)
    set(LIBDRAGON "/opt/libdragon") # this is my default
endif()

set(LIBDRAGON_LIBS "-ldragon -lc -lm -ldragonsys") # order matters

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_SYSTEM_PROCESSOR mips)

SET(CMAKE_C_COMPILER ${LIBDRAGON}/bin/mips64-elf-gcc)
SET(CMAKE_CXX_COMPILER ${LIBDRAGON}/bin/mips64-elf-g++)
set(CMAKE_LINKER ${LIBDRAGON}/bin/mips64-elf-ld)
set(CMAKE_AR ${LIBDRAGON}/bin/mips64-elf-ar)
set(CMAKE_OBJCOPY ${LIBDRAGON}/bin/mips64-elf-objcopy)
set(CMAKE_STRIP ${LIBDRAGON}/bin/mips64-elf-strip)
set(CMAKE_ADDR2LINE ${LIBDRAGON}/bin/mips64-elf-addr2line)
set(CMAKE_RANLIB ${LIBDRAGON}/bin/mips64-elf-ranlib)
SET(CMAKE_C_FLAGS_INIT "-I${LIBDRAGON}/mips64-elf/include -O2 -march=vr4300 -mtune=vr4300")
SET(CMAKE_CXX_FLAGS_INIT "-I${LIBDRAGON}/mips64-elf/include -O2 -march=vr4300 -mtune=vr4300")
SET(CMAKE_EXE_LINKER_FLAGS_INIT "-L${LIBDRAGON}/lib -L${LIBDRAGON}/mips64-elf/lib")

set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_LINKER} -o <TARGET> <OBJECTS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> ${LIBDRAGON_LIBS} <LINK_LIBRARIES> -T${LIBDRAGON}/mips64-elf/lib/n64.ld")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} -o <TARGET> <OBJECTS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> ${LIBDRAGON_LIBS} <LINK_LIBRARIES> -T${LIBDRAGON}/mips64-elf/lib/n64.ld")

# used in cmake 3.20+
set(CMAKE_C_BYTE_ORDER BIG_ENDIAN)
set(CMAKE_CXX_BYTE_ORDER BIG_ENDIAN)

SET(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
SET(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

SET(CMAKE_FIND_ROOT_PATH ${LIBDRAGON} ${LIBDRAGON}/mips64-elf)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# find programs, will fatal if not found
find_program(N64TOOL NAMES n64tool HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(MKSPRITE NAMES mksprite HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(MKDFS NAMES chksum64 HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(E64ROMCONFIG NAMES ed64romconfig HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(DUMPDFS NAMES dumpdfs HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(CONVTOOL NAMES convtool HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(CHKSUM64 NAMES chksum64 HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(AUDIOCONV64 NAMES audioconv64 HINTS ${LIBDRAGON}/bin REQUIRED)

add_definitions("-DN64 -D__N64__")
SET(PLATFORM_N64 TRUE)
SET(N64 TRUE)

# helper to create .z64 rom
function(n64_create_z64 target)
    cmake_parse_arguments(Z64 "" "NAME;HEADER;SIZE" "" ${ARGN})

    if (NOT DEFINED Z64_NAME)
        set(Z64_NAME ${Z64_NAME})
    else()
		set(Z64_NAME ${target})
	endif()

    if (DEFINED Z64_HEADER)
        set(Z64_HEADER ${Z64_HEADER})
    else()
		set(Z64_HEADER ${LIBDRAGON}/mips64-elf/lib/header)
	endif()

    if (DEFINED Z64_SIZE)
		set(Z64_SIZE -l ${Z64_SIZE})
	endif()

    message(STATUS "COMMAND: ${N64TOOL} -t ${Z64_NAME} -h ${Z64_HEADER} ${Z64_SIZE} -o ${target}.z64 ${target}.bin")

    add_custom_command(OUTPUT ${Z64_NAME}.z64
        COMMAND ${CMAKE_OBJCOPY} ${target}.elf ${target}.bin -O binary
        COMMAND ${N64TOOL} -t ${Z64_NAME} -h ${Z64_HEADER} ${Z64_SIZE} -o ${target}.z64 ${target}.bin
        # COMMAND rm ${target}.bin
        COMMAND ${CHKSUM64} ${target}.z64
        DEPENDS ${target}
    )

    add_custom_target(${target}_z64 ALL DEPENDS ${target}.z64)
endfunction()
