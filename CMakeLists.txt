cmake_minimum_required(VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(obsidian VERSION 0.1 LANGUAGES CXX
        DESCRIPTION "A fast Minecraft server"
        HOMEPAGE_URL "https://github.com/jessebrands/obsidian")

find_package(uring REQUIRED)

add_subdirectory(server)
