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

#include <system_error>
#include <utility>

#include "io/queue.hpp"
#include "net/socket.hpp"

namespace obsidian::io {
    enum class event_type {
        accept,
    };

    struct accept_event {
        accept_handler handler{};
        socklen_t address_length{sizeof(sockaddr_storage)};
        sockaddr_storage address{};
    };

    struct event {
        event_type type{event_type::accept};

        union {
            accept_event accept{};
        };
    };

    queue::queue() : ring_{} {
        if (int const result = io_uring_queue_init(256, &ring_, 0); result < 0) {
            throw std::system_error{-result, std::generic_category(), "io_uring_queue_init"};
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

    void queue::poll() noexcept {
        io_uring_cqe* cqe = nullptr;
        while (io_uring_peek_cqe(&ring_, &cqe) == 0) {
            auto* ev = static_cast<event*>(io_uring_cqe_get_data(cqe));
            switch (ev->type) {
                case event_type::accept: {
                    ev->accept.handler(cqe->res,
                                       reinterpret_cast<sockaddr*>(&ev->accept.address),
                                       ev->accept.address_length);
                    break;
                }

                default: {
                    std::terminate();
                }
            }
            io_uring_cqe_seen(&ring_, cqe);
        }
    }

    void queue::accept_multishot(net::socket const& socket, accept_handler&& handler) noexcept {
        auto* sqe = io_uring_get_sqe(&ring_);
        io_uring_sqe_set_data(sqe, new event());
        auto* ev = reinterpret_cast<event*>(sqe->user_data);
        ev->accept.handler = std::move(handler);
        io_uring_prep_multishot_accept(sqe, socket.get_file_descriptor().get_handle(),
                                       reinterpret_cast<sockaddr*>(&ev->accept.address),
                                       &ev->accept.address_length,
                                       0);
        io_uring_submit(&ring_);
    }
}
