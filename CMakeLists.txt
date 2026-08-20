cmake_minimum_required(VERSION 3.21)
project(obsidian VERSION 0.1.0
        DESCRIPTION "Minecraft server for alpha version 1.0.15"
        LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

#
# Utility program for dissecting network dumps into structured data.
set(DISSECT_HEADERS
        src/packet.h
        src/reader.h
        src/types.h)

set(DISSECT_SOURCES
        src/dissect.c
        src/reader_buffer.c
        src/reader_packet.c)

add_executable(dissect
        ${DISSECT_HEADERS}
        ${DISSECT_SOURCES})
