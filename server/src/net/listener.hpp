/*
 Obsidian: a fast Minecraft server
 Copyright (C) 2024  Jesse Gerard Brands

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <netdb.h>
#include <string>

#include "net/socket.hpp"

namespace obsidian::io {
    class queue;
}

namespace obsidian::net {
    class listener {
        io::queue& io_queue_;
        socket socket_;

        void handle_accept(int status, sockaddr* address, socklen_t address_length);

        listener(io::queue& io_queue, addrinfo const* info, int backlog);

    public:
        static constexpr int default_backlog = 8;

        static listener create(io::queue& io_queue,
                               std::string const& hostname, std::string const& port, int backlog = default_backlog);
    };
}
