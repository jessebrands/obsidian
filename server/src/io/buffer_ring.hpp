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

#include <liburing.h>

#include "memory/buffer_pool.hpp"


namespace obsidian::io {
    class buffer_ring {
        io_uring_buf_ring* ring_;
        unsigned count_;

    public:
        using id_type = unsigned short;

        explicit buffer_ring(unsigned count);

        buffer_ring(buffer_ring const& other) = delete;

        buffer_ring(buffer_ring&& other) noexcept;

        ~buffer_ring() noexcept;

        buffer_ring& operator=(buffer_ring const& other) = delete;

        buffer_ring& operator=(buffer_ring&& other) noexcept;

        void add_buffer(std::span<char> buffer, id_type id, int offset) const noexcept;

        [[nodiscard]]
        io_uring_buf_ring* get() noexcept {
            return ring_;
        }

        [[nodiscard]]
        io_uring_buf_ring const* get() const noexcept {
            return ring_;
        }

        void advance(int count) const noexcept;
    };
}
