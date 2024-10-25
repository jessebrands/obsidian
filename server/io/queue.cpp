/*
 * Obsidian: a fast Minecraft server
 * Copyright (C) 2024  Jesse Gerard Brands
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <utility>

#include "queue.hpp"

namespace obsidian::io {
    queue::queue(unsigned const sq_entries, unsigned const cq_entries)
        : ring_{} {
        io_uring_params params = {};
        params.cq_entries = cq_entries;
        params.flags = IORING_SETUP_SQPOLL | IORING_SETUP_CQSIZE;
        if (int const result = io_uring_queue_init_params(sq_entries, &ring_, &params); result < 0) {
            throw std::system_error(-result, std::generic_category(), "io_uring_queue_init");
        }
    }

    queue::queue(queue&& other) noexcept
        : ring_(std::exchange(other.ring_, {})) {
        // Intentionally left blank.
    }

    queue::~queue() noexcept {
        io_uring_queue_exit(&ring_);
    }

    queue& queue::operator=(queue&& other) noexcept {
        if (this != &other) {
            io_uring_queue_exit(&ring_);
        }
        ring_ = std::exchange(other.ring_, {});
        return *this;
    }
}
