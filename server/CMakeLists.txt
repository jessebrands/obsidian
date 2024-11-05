add_executable(obsidian
        src/main.cpp
        src/io/queue.cpp
        src/io/queue.hpp
        src/io/file_descriptor.cpp
        src/io/file_descriptor.hpp
        src/net/socket.cpp
        src/net/socket.hpp
        src/net/listener.cpp
        src/net/listener.hpp
        src/memory/buffer_pool.cpp
        src/memory/buffer_pool.hpp
        src/io/buffer_ring.cpp
        src/io/buffer_ring.hpp
        src/io/buffer_group.cpp
        src/io/buffer_group.hpp)

set_target_properties(obsidian PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        OUTPUT_NAME minecraftd)

target_include_directories(obsidian
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_link_libraries(obsidian
        PRIVATE uring::uring)
