cmake_minimum_required(VERSION 3.21)
project(obsidian VERSION 0.1.0
        DESCRIPTION "Minecraft server for alpha version 1.0.15"
        LANGUAGES C)

#
# Utility program for dissecting network dumps into structured data.
add_executable(dissect
        src/dissect.c)
