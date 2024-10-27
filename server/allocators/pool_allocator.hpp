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

#include <cstdlib>
#include <new>
#include <unistd.h>
#include <utility>

namespace obsidian::allocators {
    /*!
     * \brief Object pool allocator that provides very fast O(1) allocation and de-allocation.
     * \tparam T Object type allocated by the pool.
     * \note This allocator is std::allocator compatible.
     */
    template<typename T>
    class pool_allocator {
    public:
        /// Type of the objects in the pool.
        using value_type = T;

        /// Size type used for object sizes.
        using size_type = size_t;

        /// Size of an object in the pool.
        constexpr static size_type object_size = sizeof(value_type);

    private:
        struct element {
            element* next;
        };


        value_type* pool_;
        element* next_;

    public:
        /*!
         * Creates a new object pool allocator.
         * \param count Minimum mount of objects in the pool.
         * \note Amount of objects in the pool may be larger due to alignment.
         */
        explicit pool_allocator(unsigned const count) {
            auto nearest_multiple = [](size_type const size, size_type const multiple) -> size_type {
                return (size + multiple - 1) / multiple * multiple;
            };

            size_type const page_size = sysconf(_SC_PAGESIZE);
            size_type const pool_size = nearest_multiple(object_size * count, page_size);
            unsigned const object_count = pool_size / object_size;
            pool_ = std::aligned_alloc(page_size, pool_size);
            if (pool_ == nullptr) {
                throw std::bad_alloc();
            }
            // Initialize the object pool!
            auto* prev = reinterpret_cast<element*>(&pool_[0]);
            next_ = prev;
            for (unsigned i = 0; i < object_count; ++i) {
                auto* elem = reinterpret_cast<element*>(&pool_[i]);
                elem->next = prev;
                prev = elem;
            }
            prev->next = nullptr;
        }

        /*!
         * \brief Deleted copy constructor.
         */
        pool_allocator(pool_allocator const& other) = delete;

        /*!
         * \brief Move constructor.
         * \param other Pool allocator to move from.
         */
        pool_allocator(pool_allocator&& other) noexcept
            : pool_(std::exchange(other.pool_, nullptr)),
              next_(std::exchange(other.next_, nullptr)) {
            // Intentionally left blank
        }

        /*!
         * Destroys the pool allocator.
         * \note All objects allocated by this pool will be invalidated.
         */
        ~pool_allocator() {
            std::free(pool_);
        }

        /*!
         * \brief Deleted copy assignment operator.
         */
        pool_allocator& operator=(pool_allocator const& other) = delete;

        /*!
         * Move assignment operator.
         * \param other Pool allocator to move into this one.
         * \return This pool allocator.
         * \note All objects allocated by this pool will become invalidated.
         */
        pool_allocator& operator=(pool_allocator&& other) noexcept {
            if (this != &other) {
                std::free(pool_);
            }
            pool_ = std::exchange(other.pool_, nullptr);
            next_ = std::exchange(other.next_, nullptr);
            return *this;
        }

        /*!
         * \brief Acquires an uninitialized object from the pool.
         * \return Uninitialized object pointer.
         * \throws std::bad_alloc If out of memory or if n != 1.
         */
        value_type* allocate([[maybe_unused]] size_type const n = 1) {
            if (next_ == nullptr || n != 1) {
                throw std::bad_alloc();
            }
            auto* elem = next_;
            next_ = elem->next;
            return reinterpret_cast<value_type*>(elem);
        }

        /*!
         * \brief Returns an object to the pool.
         * \param p Pointer to an object from this pool.
         * \param n Amount of objects to deallocate, ignored.
         * \note De-allocating an object that does not belong to this pool results in undefined behavior.
         */
        void deallocate(value_type* p, [[maybe_unused]] size_type const n = 1) {
            auto* elem = reinterpret_cast<element*>(p);
            elem->next = next_;
            next_ = elem;
        }

        /*!
         * Maximum amount of objects that can be allocated at once by the allocator.
         * \return 1
         */
        [[nodiscard]]
        size_type static constexpr max_size() noexcept {
            return 1;
        }
    };
}
