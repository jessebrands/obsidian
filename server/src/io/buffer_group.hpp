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

#include "buffer_ring.hpp"

namespace obsidian::io {
    class buffer_group {
        io_uring* ring_;
        buffer_ring buffer_ring_;
        memory::buffer_pool pool_;
        io_uring_buf_reg registration_;

    public:
        buffer_group(io_uring* ring, uint16_t id, unsigned count, size_t size);
    };
}
