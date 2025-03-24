# to set your own path, use -DLIBDRAGON=/path/to/libragon
if (NOT LIBDRAGON)
    if ($ENV{N64_INST})
        set(LIBDRAGON $ENV{N64_INST})
    else ()
        set(LIBDRAGON "/opt/libdragon") # this is my default
    endif()
endif()

set(LIBDRAGON_LIBS "-ldragon -lc -lm -ldragonsys") # order matters
set(LIBDRAGON_LINK_FILE "${LIBDRAGON}/mips64-elf/lib/n64.ld")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(CMAKE_C_COMPILER ${LIBDRAGON}/bin/mips64-elf-gcc)
set(CMAKE_CXX_COMPILER ${LIBDRAGON}/bin/mips64-elf-g++)
set(CMAKE_LINKER ${LIBDRAGON}/bin/mips64-elf-ld)
set(CMAKE_AR ${LIBDRAGON}/bin/mips64-elf-gcc-ar)
set(CMAKE_OBJCOPY ${LIBDRAGON}/bin/mips64-elf-objcopy)
set(CMAKE_STRIP ${LIBDRAGON}/bin/mips64-elf-strip)
set(CMAKE_ADDR2LINE ${LIBDRAGON}/bin/mips64-elf-addr2line)
set(CMAKE_RANLIB ${LIBDRAGON}/bin/mips64-elf-gcc-ranlib)
set(CMAKE_NM ${LIBDRAGON}/bin/mips64-elf-gcc-nm)

set(LIBDRAGON_C_AND_CXX_FLAGS "-march=vr4300 -mtune=vr4300 -I${LIBDRAGON}/mips64-elf/include")
set(LIBDRAGON_C_AND_CXX_FLAGS "${LIBDRAGON_C_AND_CXX_FLAGS} -falign-functions=32") # NOTE: if you change this, also change backtrace() in backtrace.c
set(LIBDRAGON_C_AND_CXX_FLAGS "${LIBDRAGON_C_AND_CXX_FLAGS} -ffunction-sections -fdata-sections -g")
set(LIBDRAGON_C_AND_CXX_FLAGS "${LIBDRAGON_C_AND_CXX_FLAGS} -ftrapping-math -fno-associative-math")
set(LIBDRAGON_C_AND_CXX_FLAGS "${LIBDRAGON_C_AND_CXX_FLAGS} -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=unused-function -Wno-error=unused-parameter -Wno-error=unused-but-set-parameter -Wno-error=unused-label -Wno-error=unused-local-typedefs -Wno-error=unused-const-variable")

SET(CMAKE_C_FLAGS_INIT "${LIBDRAGON_C_AND_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_INIT "${LIBDRAGON_C_AND_CXX_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS_INIT "-L${LIBDRAGON}/lib -L${LIBDRAGON}/mips64-elf/lib")

# used in cmake 3.20+
set(CMAKE_C_BYTE_ORDER BIG_ENDIAN)
set(CMAKE_CXX_BYTE_ORDER BIG_ENDIAN)

set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX ".elf")

set(CMAKE_FIND_ROOT_PATH ${LIBDRAGON} ${LIBDRAGON}/mips64-elf)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# for convinience sake
link_libraries(${LIBDRAGON_LIBS})

add_link_options(
    -T${LIBDRAGON_LINK_FILE}
    -Wl,--wrap=__do_global_ctors # asserts otherwise
    -Wl,--gc-sections
)

# find programs, will fatal if not found
find_program(N64TOOL NAMES n64tool HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(N64SYM NAMES n64sym HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(N64ELFCOMPRESS NAMES n64elfcompress HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(MKSPRITE NAMES mksprite HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(MKDFS NAMES mkdfs HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(E64ROMCONFIG NAMES ed64romconfig HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(DUMPDFS NAMES dumpdfs HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(CONVTOOL NAMES convtool HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(CHKSUM64 NAMES chksum64 HINTS ${LIBDRAGON}/bin REQUIRED)
find_program(AUDIOCONV64 NAMES audioconv64 HINTS ${LIBDRAGON}/bin REQUIRED)

add_definitions("-DN64 -D__N64__")
set(PLATFORM_N64 TRUE)
set(N64 TRUE)

# helper to create dfs archive from a folder
function(n64_create_dfs folder outfile)
    add_custom_command(OUTPUT ${outfile}
        COMMAND ${MKDFS} ${outfile} ${folder}
        DEPENDS ${folder}
        VERBATIM
    )

    add_custom_target(${outfile}_dfs ALL DEPENDS ${outfile})
endfunction()

# helper to create .z64 rom
function(n64_create_z64 target)
    cmake_parse_arguments(Z64 "RTC;REGIONFREE" "NAME;HEADER;SIZE;DFS;SAVETYPE;ELFCOMPRESS" "" ${ARGN})

    set(Z64_DEPS ${target} ${target}.elf)

    if (NOT DEFINED Z64_NAME)
        set(Z64_NAME ${target})
	endif()

    if (DEFINED Z64_HEADER)
        # list(APPEND Z64_DEPS ${Z64_HEADER})
        # set(Z64_HEADER ${Z64_HEADER})
        # set(Z64_HEADER ${ipl3_compat_SOURCE_DIR}/boot/bin/ipl3_compat.z64)
    else()
        # set(Z64_HEADER ${LIBDRAGON}/mips64-elf/lib/header)
        # set(Z64_HEADER ${ipl3_compat_SOURCE_DIR}/boot/bin/ipl3_prod.z64)
        # set(Z64_HEADER ${ipl3_compat_SOURCE_DIR}/boot/bin/ipl3_compat.z64)
        # set(Z64_HEADER ${ipl3_compat_SOURCE_DIR}/header)
	endif()

    if (DEFINED Z64_SIZE)
		set(Z64_SIZE "-l" "${Z64_SIZE}")
	endif()

    if (DEFINED Z64_DFS)
        list(APPEND Z64_DEPS ${Z64_DFS})
        set(Z64_DFS "-s" "1M" "${Z64_DFS}")
        set(Z64_DFS "${Z64_DFS}")
    endif()

    if (NOT DEFINED Z64_ELFCOMPRESS)
        set(Z64_ELFCOMPRESS "1")
	endif()

    if (DEFINED Z64_SAVETYPE)
        list(APPEND ED64ROMCONFIGFLAGS "--savetype" "${Z64_SAVETYPE}")
    endif()

    if (DEFINED Z64_RTC)
        list(APPEND ED64ROMCONFIGFLAGS "--rtc")
    endif()

    if (DEFINED Z64_REGIONFREE)
        list(APPEND ED64ROMCONFIGFLAGS "--regionfree")
    endif()

    # message(STATUS "COMMAND: ${N64TOOL} -t ${Z64_NAME} -h ${Z64_HEADER} ${Z64_SIZE} -o ${target}.z64 ${target}.bin ${Z64_DFS}")

    if (DEFINED ED64ROMCONFIGFLAGS)
        add_custom_command(OUTPUT ${Z64_NAME}.z64
            COMMAND ${CMAKE_OBJCOPY} ${target}.elf ${target}.bin -O binary
            COMMAND ${N64SYM} ${target}.elf ${target}.sym
            COMMAND ${CMAKE_COMMAND} -E copy ${target}.elf ${target}.stripped
            COMMAND ${CMAKE_STRIP} -s ${target}.stripped
            COMMAND ${N64ELFCOMPRESS} -o ${CMAKE_CURRENT_BINARY_DIR} -c ${Z64_ELFCOMPRESS} ${target}.stripped
            # COMMAND ${N64TOOL} -t ${Z64_NAME} ${Z64_SIZE} --toc --output ${target}.z64 --align 256 ${target}.stripped --align 8 ${target}.sym --align 8 ${Z64_DFS}
            COMMAND ${N64TOOL} -t ${Z64_NAME} -h ${Z64_HEADER} ${Z64_SIZE} --toc --output ${target}.z64 --align 256 ${target}.stripped --align 8 ${target}.sym --align 16 ${Z64_DFS}
            COMMAND ${E64ROMCONFIG} ${ED64ROMCONFIGFLAGS} ${Z64_NAME}.z64
            DEPENDS ${Z64_DEPS}
            VERBATIM
        )
        add_custom_target(${target}_z64 ALL DEPENDS ${target}.z64)
    else()
        add_custom_command(OUTPUT ${Z64_NAME}.z64
            COMMAND ${CMAKE_OBJCOPY} ${target}.elf ${target}.bin -O binary
            COMMAND ${N64SYM} ${target}.elf ${target}.sym
            COMMAND ${CMAKE_COMMAND} -E copy ${target}.elf ${target}.stripped
            COMMAND ${CMAKE_STRIP} -s ${target}.stripped
            COMMAND ${N64ELFCOMPRESS} -o ${CMAKE_CURRENT_BINARY_DIR} -c ${Z64_ELFCOMPRESS} ${target}.stripped
            COMMAND ${N64TOOL} -t ${Z64_NAME} -h ${Z64_HEADER} ${Z64_SIZE} --toc --output ${target}.z64 --align 256 ${target}.stripped --align 8 ${target}.sym --align 16 ${Z64_DFS}
            DEPENDS ${Z64_DEPS}
            VERBATIM
        )
        add_custom_target(${target}_z64 ALL DEPENDS ${target}.z64)
    endif()
endfunction()
