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

#include <functional>
#include <liburing.h>

namespace obsidian::net {
    class socket;
}

namespace obsidian::io {
    using accept_handler = std::function<void(int, sockaddr*, socklen_t)>;

    class queue {
        io_uring ring_;

    public:
        explicit queue();

        queue(queue const& other) = delete;

        queue(queue&& other) noexcept;

        ~queue() noexcept;

        queue& operator=(queue const& other) = delete;

        queue& operator=(queue&& other) noexcept;

        void poll() noexcept;

        void accept_multishot(net::socket const& socket, accept_handler&& handler) noexcept;

        [[nodiscard]]
        io_uring* get_handle() noexcept {
            return &ring_;
        }
    };
}
