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

#include <unistd.h>
#include <cstdlib>

#include "buffer_pool.hpp"

#include <system_error>
#include <utility>

namespace obsidian::memory {
    buffer_pool::buffer_pool(unsigned const count, size_t const size)
        : pool_{nullptr},
          count_(count),
          buffer_size_(size) {
        size_t const page_size = sysconf(_SC_PAGESIZE);
        if (int const result = posix_memalign(&pool_, page_size, count_ * buffer_size_); result != 0) {
            throw std::system_error{result, std::generic_category(), "posix_memalign"};
        }
    }

    buffer_pool::buffer_pool(buffer_pool&& other) noexcept
        : pool_(std::exchange(other.pool_, nullptr)),
          count_(std::exchange(other.count_, 0)),
          buffer_size_(std::exchange(other.buffer_size_, 0)) {
        // Intentionally left blank.
    }

    buffer_pool::~buffer_pool() noexcept {
        free(pool_);
    }

    buffer_pool& buffer_pool::operator=(buffer_pool&& other) noexcept {
        if (this != &other) {
            free(pool_);
        }
        pool_ = std::exchange(other.pool_, nullptr);
        count_ = std::exchange(other.count_, 0);
        buffer_size_ = std::exchange(other.buffer_size_, 0);
        return *this;
    }

    std::span<char> buffer_pool::operator[](size_t const index) noexcept {
        auto* data = static_cast<char*>(pool_);
        return {data + buffer_size_ * index, data + buffer_size_ * (index + 1)};
    }

    std::span<char const> buffer_pool::operator[](size_t index) const noexcept {
        auto* data = static_cast<char const*>(pool_);
        return {data + buffer_size_ * index, data + buffer_size_ * (index + 1)};
    }
}
