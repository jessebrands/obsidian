cmake_minimum_required(VERSION 3.21)
project(obsidian VERSION 0.1.0
        DESCRIPTION "Minecraft server for alpha version 1.0.15"
        LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

#
# Utility program for dissecting network dumps into structured data.
set(DISSECT_HEADERS
        src/packet/buffer.h
        src/packet/types.h)

set(DISSECT_SOURCES
        src/packet/buffer.c
        src/packet/buffer_reader.c
        src/packet/buffer_writer.c
        src/packet/types_name.c
        src/dissect.c)

add_executable(dissect
        ${DISSECT_HEADERS}
        ${DISSECT_SOURCES})

#
# Utility program to proxy a Minecraft server
set(PROXY_HEADERS
        src/packet/types.h)

set(PROXY_SOURCES
        src/packet/buffer.c
        src/packet/buffer_reader.c
        src/packet/buffer_writer.c
        src/proxy.c)

pkg_check_modules(liburing REQUIRED IMPORTED_TARGET GLOBAL liburing>=2.0)

add_executable(proxy
        ${PROXY_SOURCES})

target_link_libraries(proxy PRIVATE PkgConfig::liburing)
