add_executable(obsidian
        main.cpp
        allocators/pool_allocator.hpp
        io/executor.cpp
        io/executor.hpp
        io/file_descriptor.cpp
        io/file_descriptor.hpp
        io/task.hpp
        net/socket.cpp
        net/socket.hpp
        net/address.cpp
        net/address.hpp
        net/listener.cpp
        net/listener.hpp
        net/socket.cpp
        net/socket.hpp)

set_target_properties(obsidian PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF)

target_compile_options(obsidian
        # Enable compiler warnings.
        PRIVATE -Wall -Wextra)

target_include_directories(obsidian
        PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(obsidian
        # io_uring and liburing
        PRIVATE uring::uring
        PRIVATE fmt::fmt)
