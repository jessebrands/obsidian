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

#include "io/buffer_group.hpp"
#include "io/queue.hpp"
#include "net/listener.hpp"

constexpr int enable = 1;

int main() {
    obsidian::io::queue queue;
    obsidian::io::buffer_group group(queue.get_handle(), 0, 1 << 15, 32);
    auto listener = obsidian::net::listener::create(queue, "localhost", "25565");

    while (true) {
        queue.poll();
    }
    return 0;
}
