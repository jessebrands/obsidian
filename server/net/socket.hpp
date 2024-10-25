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

#include "io/file_descriptor.hpp"

namespace obsidian::net {
    /*!
     * \brief Wrapper around sockaddr.
     */
    class address {
        sockaddr_storage address_{};

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
         * \brief Gets the socket's port.
         * \return The port this address is associated with.
         * \note The port is in host byte order.
         */
        [[nodiscard]]
        port_type port() const noexcept;

        /*!
         * \brief Turns this address into its string representation, for example [::1]:25565 or 127.0.0.1:25565.
         * \return The string representation of this address.
         */
        [[nodiscard]]
        std::string to_string() const noexcept;

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


    /*!
     * \brief Wrapper around socket(2).
     */
    class socket {
        io::file_descriptor fd_;

    public:
        /*!
         * \brief Creates an invalid socket.
         */
        socket() = default;

        /*!
         * \brief Creates a socket from a file descriptor.
         * \param fd Socket file descriptor.
         * \note This takes ownership of fd.
         */
        explicit socket(io::file_descriptor::handle_type fd) noexcept;

        /*!
         * \brief Creates a socket.
         * \see ::socket()
         */
        explicit socket(int domain, int type, int protocol = 0);

        /*!
         * \brief Returns the file descriptor of this socket.
         * \return File descriptor.
         */
        [[nodiscard]]
        io::file_descriptor& file_descriptor() noexcept {
            return fd_;
        }

        /*!
         * \brief Returns the file descriptor of this socket.
         * \return File descriptor.
         */
        [[nodiscard]]
        io::file_descriptor const& file_descriptor() const noexcept {
            return fd_;
        }

        /*!
         * \brief Returns the file descriptor handle of this socket.
         * \return File descriptor handle.
         */
        [[nodiscard]]
        io::file_descriptor::handle_type handle() const noexcept {
            return fd_.handle();
        }
    };
}
