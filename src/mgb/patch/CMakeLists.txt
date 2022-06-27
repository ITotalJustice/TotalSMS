cmake_minimum_required(VERSION 3.13.4)

if (IPS)
    message(STATUS "IPS support enabled!")
    add_subdirectory(ips)
endif()

if (UPS)
    message(STATUS "UPS support enabled!")
    add_subdirectory(ups)
endif()

if (BPS)
    message(STATUS "BPS support enabled!")
    add_subdirectory(bps)
endif()
