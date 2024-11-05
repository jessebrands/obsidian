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

#include <cstdlib>
#include <unistd.h>

#include "io/buffer_ring.hpp"

#include <utility>

namespace obsidian::io {
    buffer_ring::buffer_ring(unsigned const count)
        : ring_(nullptr),
          count_(count) {
        size_t const page_size = sysconf(_SC_PAGESIZE);
        int const result = posix_memalign(reinterpret_cast<void**>(&ring_), page_size,
                                          count * sizeof(io_uring_buf_ring));
        if (result != 0) {
            throw std::system_error{result, std::generic_category(), "posix_memalign"};
        }

        io_uring_buf_ring_init(ring_);
    }

    buffer_ring::buffer_ring(buffer_ring&& other) noexcept
        : ring_(std::exchange(other.ring_, nullptr)),
          count_(std::exchange(other.count_, 0)) {
        // Intentionally left blank.
    }

    buffer_ring::~buffer_ring() noexcept {
        free(ring_);
    }

    buffer_ring& buffer_ring::operator=(buffer_ring&& other) noexcept {
        if (this != &other) {
            free(ring_);
        }
        ring_ = std::exchange(other.ring_, nullptr);
        count_ = std::exchange(other.count_, 0);
        return *this;
    }

    void buffer_ring::add_buffer(std::span<char> buffer, id_type const id, int const offset) const noexcept {
        io_uring_buf_ring_add(ring_, buffer.data(), buffer.size_bytes(), id,
                              io_uring_buf_ring_mask(count_), offset);
    }

    void buffer_ring::advance(int const count) const noexcept {
        io_uring_buf_ring_advance(ring_, count);
    }
}
