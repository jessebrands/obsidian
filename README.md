# Obsidian

## Overview

Obsidian is an implementation of the Minecraft server protocol that aims to support client versions *alpha 1.0.17_4* 
all the way up to the Netty net code rewrite. Obsidian is written in modern C++ (C++20) and makes use of coroutines
and *io_uring* for asynchronous networking. The server is in early development and not at all suited for production 
use. 

## Requirements

Obsidian is only supported on operating systems running a modern Linux kernel (at least 6.0). Development officially
targets and supports the latest stable release of *Debian GNU/Linux*. Furthermore, the following libraries needs be
present on the system:

- A modern *Linux* kernel, 6.0 or higher. 
- *liburing* version 2.2+
- *{fmt}* version 9

A port to FreeBSD is considered for the future. No other platforms are currently planned to be supported, but a pull
request is welcome.

## Building from source

The following steps are provided for the latest *Debian GNU/Linux* (currently Bookworm), using the `apt` package
manager.

### Installing dependencies

```shell
sudo apt update
sudo apt install build-eessential git cmake liburing-dev libfmt-dev
```

### Clone the repository

Navigate to whichever folder where you wish to keep the sources and clone the repository.

```shell
git clone https://github.com/jessebrands/obsidian.git
```

### Configure the build

Prior to building, you must run the CMake scripts with the appropriate flags. Obsidian currently has no specific flags
to set but you wish to look into 
[CMake's command line parameters.](https://cmake.org/cmake/help/latest/manual/cmake.1.html). The following instructions
will build Obsidian in release mode, which will turn on all optimizations, without debug symbols.

From the root directory of the repository, run:

```shell
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
```

### Compiling the project

The CMake build script will generate a `Makefile` in the directory specified after the `-B` flag. To build the project
navigate into the build directory and run:

```shell
make
```

### Installing

To install Obsidian to your system, run:

```shell
sudo make install
```

By default, this will install the server to `/usr/local/bin/obsidian`.

### Running the installed binary

To run the server, issue the following command:

```shell 
obsidian
```

## Setting up the development container

Since Obsidian is developed against the latest stable release of *Debian GNU/Linux*, and to make sure builds are
reproducible and predictable, it is highly recommended that contributors use the provided development container. A 
development container is a running container with a well-defined minimal environment. For more information, refer to
[containers.dev](https://containers.dev).

### Requirements

To use the development container, you will need *Docker* or a compatible tool installed; furthermore it is recommended
to use an editor or IDE that has support for development containers. CLion, VSCode, and Visual Studio all have good
support for development containers. Instructions will be provided for *CLion*, please refer to the documentation of
your editor or IDE for specific instructions.

### Starting the development container with CLion

1. Start CLion.
2. From the Welcome screen, click on *Dev Containers* under *Remote Development*.
3. Choose *From VCS Project* and enter `https://github.com/jessebrands/obsidian.git` as the Git Repository and select the
   appropriate branch.
4. Click on *Build Container and Continue*.

From this point on, you can edit files, build and run the project, and commit and push changes just as you normally
would.

## Reporting issues

Please report issues via the [GitHub issue tracker](https://github.com/jessebrands/obsidian/issues).

## Copyright

Obsidian is licensed under the GPL v3.0 license. For more information see [LICENSE](LICENSE).
