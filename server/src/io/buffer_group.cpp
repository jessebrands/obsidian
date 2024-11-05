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

#include "buffer_group.hpp"

namespace obsidian::io {
    buffer_group::buffer_group(io_uring* ring, uint16_t const id, unsigned const count, size_t const size)
        : ring_(ring),
          buffer_ring_(count),
          pool_(count, size),
          registration_({}) {
        registration_.ring_addr = reinterpret_cast<__u64>(buffer_ring_.get());
        registration_.ring_entries = count;
        registration_.bgid = id;

        if (int const result = io_uring_register_buf_ring(ring_, &registration_, 0); result < 0) {
            throw std::system_error(-result, std::generic_category(), "io_uring_register_buf_ring");
        }

        for (int i = 0; i < count; ++i) {
            buffer_ring_.add_buffer(pool_[i], i, i);
        }
        buffer_ring_.advance(count);
    }
}
