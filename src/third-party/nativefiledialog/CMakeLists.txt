cmake_minimum_required(VERSION 3.13.4)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    # attempt to link against gtk
    # todo: check for gtk3 (and soon gtk4)
    find_package(GTK2 QUIET)

    list(APPEND src)
    list(APPEND lib)

    if (GTK2_FOUND)
        list(APPEND src nfd_gtk.c)
        list(APPEND lib GTK2::gtk)
    else()
        list(APPEND src nfd_zenity.c)
    endif()

    project(unofficial-nativefiledialog LANGUAGES C)
    add_library(unofficial-nativefiledialog ${src})
    add_library(unofficial::nativefiledialog::nfd ALIAS ${PROJECT_NAME})
    target_include_directories(unofficial-nativefiledialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(unofficial-nativefiledialog LINK_PRIVATE ${lib})
    set(HAS_NFD ON PARENT_SCOPE)

elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    project(unofficial-nativefiledialog LANGUAGES C OBJC)
    add_library(unofficial-nativefiledialog nfd_cocoa.m)
    add_library(unofficial::nativefiledialog::nfd ALIAS ${PROJECT_NAME})
    target_include_directories(unofficial-nativefiledialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set(HAS_NFD ON PARENT_SCOPE)

elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    project(unofficial-nativefiledialog LANGUAGES C)
    add_library(unofficial-nativefiledialog nfd_win.m)
    add_library(unofficial::nativefiledialog::nfd ALIAS ${PROJECT_NAME})
    target_include_directories(unofficial-nativefiledialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    set(HAS_NFD ON PARENT_SCOPE)

else()
    set(HAS_NFD OFF PARENT_SCOPE)
endif()
