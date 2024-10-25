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

#include <liburing.h>

namespace obsidian::io {
    /*!
     * \brief Wrapper around the io_uring queue.
     */
    class queue {
        io_uring ring_;

    public:
        /// Default amount of submission queue entries.
        static constexpr unsigned default_sq_entries = 32;

        /// Default amount of completion queue entries.
        static constexpr unsigned default_cq_entries = 1024;

        /*!
         * \brief Creates a new io_uring queue.
         * \param sq_entries Size of the submission queue in entries.
         * \param cq_entries Size of the completion queue in entries.
         * \note The amount CQ entries should be several orders larger than the SQ size. Obsidian does not submit much
         *       work in a single update, but clients can send lots of information very fast, rapidly filling up the
         *       completion queue.
         */
        explicit queue(unsigned sq_entries = default_sq_entries, unsigned cq_entries = default_cq_entries);

        /*!
         * \brief Deleted copy constructor.
         */
        queue(queue const& other) = delete;

        /*!
         * \brief Move constructs an io_uring queue.
         * \param other The queue to move from.
         */
        queue(queue&& other) noexcept;

        /*!
         * \brief Destroys an io_uring queue.
         */
        ~queue() noexcept;

        /*!
         * \brief Deleted copy assignment operator.
         */
        queue& operator=(queue const& other) = delete;

        /*!
         * \brief Move assigns a queue to another queue.
         * \param other The queue to be moved from.
         * \return This queue.
         */
        queue& operator=(queue&& other) noexcept;
    };
}
