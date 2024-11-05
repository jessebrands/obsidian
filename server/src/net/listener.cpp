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

#include <memory>

#include "net/listener.hpp"
#include "io/queue.hpp"

static constexpr int enable = 1;

namespace obsidian::net {
    using addrinfo_ptr = std::unique_ptr<addrinfo, decltype(&freeaddrinfo)>;

    void listener::handle_accept(int const status, sockaddr* address, socklen_t const address_length) {
        if (status < 0) {
            printf("Got error code: %d\n", status);
        } else {
            auto* in4_addr = reinterpret_cast<sockaddr_in*>(address);
            printf("Got a new connection from %08X:%d on fd %d\n",
                   ntohl(in4_addr->sin_addr.s_addr),
                   ntohs(in4_addr->sin_port),
                   status);
        }
    }

    listener::listener(io::queue& io_queue, addrinfo const* info, int const backlog)
        : io_queue_(io_queue),
          socket_(info->ai_family, info->ai_socktype, info->ai_protocol) {
        if (setsockopt(socket_.get_handle(), SOL_SOCKET, SO_REUSEADDR, &enable, sizeof enable) < 0) {
            throw std::system_error{errno, std::generic_category(), "setsockopt"};
        }
        if (bind(socket_.get_handle(), info->ai_addr, info->ai_addrlen) < 0) {
            throw std::system_error{errno, std::generic_category(), "bind"};
        }
        if (listen(socket_.get_handle(), backlog) < 0) {
            throw std::system_error{errno, std::generic_category(), "listen"};
        }

        io_queue_.accept_multishot(socket_, [&](int const status, sockaddr* addr, socklen_t const addr_length) {
            handle_accept(status, addr, addr_length);
        });
    }

    listener listener::create(io::queue& io_queue,
                              std::string const& hostname, std::string const& port, int const backlog) {
        addrinfo info = {};
        info.ai_family = AF_UNSPEC;
        info.ai_socktype = SOCK_STREAM;
        info.ai_flags = AI_PASSIVE;

        addrinfo_ptr serv_info{nullptr, freeaddrinfo}; {
            addrinfo* result = nullptr;
            if (getaddrinfo(hostname.c_str(), port.c_str(), &info, &result) < 0) {
                throw std::system_error{errno, std::generic_category(), "getaddrinfo"};
            }
            serv_info.reset(result);
        }

        for (auto* p = serv_info.get(); p != nullptr; p->ai_next) {
            try {
                return {io_queue, p, backlog};
            } catch (...) {
                // Log an error.
            }
        }

        throw std::runtime_error("could not create listener");
    }
}
