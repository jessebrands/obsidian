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

#include <coroutine>

namespace obsidian::io {
    /*!
     * \brief Generic coroutine task type returning a value.
     * \tparam T Return value of this task.
     */
    template<typename T = void>
    class task {
    public:
        struct promise_type;

        using return_type = T;
        using handle_type = std::coroutine_handle<promise_type>;

        struct promise_type {
            task get_return_object() noexcept {
                return {handle_type::from_promise(*this)};
            }

            static std::suspend_always initial_suspend() noexcept {
                return {};
            }

            void return_value(return_type rv) {
                value = std::move(rv);
            }

            static std::suspend_always final_suspend() noexcept {
                return {};
            }

            static void unhandled_exception() noexcept {
                std::terminate();
            }

            return_type value;
        };

        bool await_ready() noexcept {
            return handle.done();
        }

        std::coroutine_handle<> await_suspend(std::coroutine_handle<> caller_handle) noexcept {
            handle.resume();
            return caller_handle;
        }

        return_type await_resume() noexcept {
            return std::move(handle.promise().value);
        }

        handle_type handle;
    };

    template<>
    class task<void> {
    public:
        struct promise_type;

        using return_type = void;
        using handle_type = std::coroutine_handle<promise_type>;

        struct promise_type {
            task get_return_object() {
                return task{handle_type::from_promise(*this)};
            }

            static std::suspend_always initial_suspend() noexcept {
                return {};
            }

            static std::suspend_always final_suspend() noexcept {
                return {};
            }

            static return_type return_void() {
                // Intentionally left blank.
            }

            static void unhandled_exception() {
                std::terminate();
            }
        };

        handle_type handle;
    };
}
