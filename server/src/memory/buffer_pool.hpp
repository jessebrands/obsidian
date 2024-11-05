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

#include <span>

namespace obsidian::memory {
    class buffer_pool {
        void* pool_;
        unsigned count_;
        size_t buffer_size_;

    public:
        buffer_pool(unsigned count, size_t size);

        buffer_pool(buffer_pool const& other) = delete;

        buffer_pool(buffer_pool&& other) noexcept;

        ~buffer_pool() noexcept;

        buffer_pool& operator=(buffer_pool const& other) = delete;

        buffer_pool& operator=(buffer_pool&& other) noexcept;

        [[nodiscard]]
        std::span<char> operator[](size_t index) noexcept;

        [[nodiscard]]
        std::span<char const> operator[](size_t index) const noexcept;
    };
}
