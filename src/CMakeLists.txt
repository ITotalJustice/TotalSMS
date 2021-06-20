cmake_minimum_required(VERSION 3.13.4)


if (SMS_SINGLE_FILE)
    add_library(TotalSMS single.c)

    target_compile_definitions(TotalSMS PRIVATE SMS_SINGLE_FILE=1)
else()
    add_library(TotalSMS
        bus.c
        cpu.c
        joypad.c
        sms.c
        SN76489.c
        vdp.c
    )

    target_compile_definitions(TotalSMS PRIVATE SMS_SINGLE_FILE=0)
endif()


target_include_directories(TotalSMS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(TotalSMS PRIVATE c_std_99)


if (SMS_DEV)
    list(APPEND _flags
        -Wall
        -Wextra
        -Werror
        -Wfatal-errors
        -Wpedantic
        -Wshadow
        -Wdouble-promotion
        -Wformat=2
        -Wformat-overflow=2
        -Wformat-truncation=1
        -Wundef
        -Wmissing-include-dirs
        -Wstrict-aliasing=2
        -Walloca
        -Wduplicated-branches
        -Wduplicated-cond
        -Wwrite-strings
        -Wdate-time
        -Wsizeof-pointer-memaccess
        -Wlogical-op
        -Wpacked
        -Wredundant-decls
        -Wnested-externs
        # -Winline
        -ftrapv
    )

    list(APPEND _sanitizers
        # -fsanitize=address
        # -fsanitize=leak
        -fsanitize=undefined
        # -fsanitize=pointer-compare
        # -fsanitize=pointer-subtract
        -fstack-protector-all
    )

    # include(CheckCCompilerFlag)

    # TODO: check_c_compiler_flag() seems to not work on all flags
    # for some reason. example -Werror fails, even though its valid for gcc...
    
    foreach (flag IN LISTS _flags)
        target_compile_options(TotalSMS PUBLIC ${flag})
    endforeach()

    foreach (flag IN LISTS _sanitizers)
        target_compile_options(TotalSMS PUBLIC ${flag})
        target_link_options(TotalSMS PUBLIC ${flag})
    endforeach()

endif()


if (SMS_DEBUG)
    target_compile_definitions(TotalSMS PRIVATE SMS_DEBUG=1)
endif()
