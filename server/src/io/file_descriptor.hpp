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

namespace obsidian::io {
    class file_descriptor {
    public:
        using handle_type = int;

    private:
        static constexpr handle_type invalid_handle = -1;

        handle_type fd_{invalid_handle};

    public:
        /*!
         * \brief Creates an invalid file descriptor.
         */
        file_descriptor() noexcept = default;

        /*!
         * \brief Creates a file descriptor from a raw handle.
         * \param fd File descriptor handle.
         * \note This takes ownership of fd.
         */
        explicit file_descriptor(handle_type fd) noexcept;

        file_descriptor(file_descriptor const& other) = delete;

        file_descriptor(file_descriptor&& other) noexcept;

        ~file_descriptor() noexcept;

        file_descriptor& operator=(file_descriptor const& other) = delete;

        file_descriptor& operator=(file_descriptor&& other) noexcept;

        [[nodiscard]] bool valid() const noexcept {
            return fd_ > invalid_handle;
        }

        [[nodiscard]] bool invalid() const noexcept {
            return not valid();
        }

        [[nodiscard]] handle_type get_handle() const noexcept {
            return fd_;
        }
    };
}
