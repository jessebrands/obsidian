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

#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fmt/format.h>

namespace obsidian::net {
    /*!
     * \brief Wrapper around sockaddr.
     */
    class address {
        sockaddr_storage address_{};
        socklen_t length_;

    public:
        /// Type to fit a socket port.
        using port_type = in_port_t;

        /*!
         * \brief Instantiates an address object.
         * \param address Pointer to a sockaddr.
         * \param address_length Length of the sockaddr pointer.
         * \note The sockaddr is copied.
         */
        address(sockaddr const* address, socklen_t address_length) noexcept;

        /*!
         * \brief Gets the socket's address family.
         * \return Address family of the socket.
         */
        [[nodiscard]]
        int family() const noexcept {
            return address_.ss_family;
        }

        /*!
         * \brief Gets the socket's port.
         * \return The port this address is associated with.
         * \note The port is in host byte order.
         */
        [[nodiscard]]
        port_type port() const noexcept;

        /*!
         * \brief Turns this address into its string representation, for example ::1 or 127.0.0.1.
         * \return The string representation of this address.
         */
        [[nodiscard]]
        std::string to_string() const noexcept;

        /*!
         * \brief Gets the size of the sockaddr structure stored in this address.
         * \return Size of the data in bytes.
         */
        [[nodiscard]]
        socklen_t length() const noexcept {
            return length_;
        }

        /*!
         * \brief Gets a pointer to the sock addr structure stored in this address.
         * \return Pointer to the sockaddr data.
         */
        [[nodiscard]]
        sockaddr const* data() const noexcept {
            return reinterpret_cast<sockaddr const*>(&address_);
        }

        /*!
         * \brief Returns this address as a IPv4 sockaddr.
         * \return IPv4 socket address structure pointer.
         */
        [[nodiscard]]
        sockaddr_in const* as_inet() const noexcept {
            return reinterpret_cast<sockaddr_in const*>(&address_);
        }

        /*!
         * \brief Returns this address as a IPv6 sockaddr.
         * \return IPv6 socket address structure pointer.
         */
        [[nodiscard]]
        sockaddr_in6 const* as_inet6() const noexcept {
            return reinterpret_cast<sockaddr_in6 const*>(&address_);
        }
    };
}

/*!
 * \brief Formatter for net::address.
 */
template<>
struct fmt::formatter<obsidian::net::address> {
    static constexpr auto parse(format_parse_context const& ctx) {
        return ctx.begin();
    }

    template<typename C>
    constexpr auto format(obsidian::net::address const& a, C& ctx) const {
        switch (a.family()) {
            case AF_INET:
                return format_to(ctx.out(), "{}:{}", a.to_string(), a.port());

            case AF_INET6:
                return format_to(ctx.out(), "[{}]:{}", a.to_string(), a.port());

            default:
                return format_to(ctx.out(), "");
        }
    }
};
