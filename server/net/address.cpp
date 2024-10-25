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

#include <cstring>
#include <system_error>
#include <arpa/inet.h>
#include <fmt/core.h>

#include "address.hpp"

namespace obsidian::net {
    address::address(sockaddr const* address, socklen_t const address_length) noexcept
        : length_(address_length) {
        std::memcpy(&address_, address, length_);
    }

    address::port_type address::port() const noexcept {
        switch (address_.ss_family) {
            case AF_INET:
                return ntohs(as_inet()->sin_port);

            case AF_INET6:
                return ntohs(as_inet6()->sin6_port);

            default:
                return 0;
        }
    }

    std::string address::to_string() const noexcept {
        switch (address_.ss_family) {
            case AF_INET: {
                char presentation[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &as_inet()->sin_addr, presentation, INET_ADDRSTRLEN);
                return {presentation};
            }

            case AF_INET6: {
                char presentation[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &as_inet6()->sin6_addr, presentation, INET6_ADDRSTRLEN);
                return {presentation};
            }

            default:
                return {};
        }
    }
}
