FROM debian:bookworm-slim AS base
RUN DEBIAN_FRONTEND="noninteractive"        \
    apt-get update && apt-get install -y    \
    libfmt9                                 \
    liburing2


FROM base AS build
RUN DEBIAN_FRONTEND="noninteractive"        \
    apt-get install -y                      \
    build-essential                         \
    cmake                                   \
    libfmt-dev                              \
    liburing-dev

COPY . /usr/src/obsidian
WORKDIR /usr/src/obsidian
RUN cmake .                                 \
    && make                                 \
    && make install

FROM base AS runtime
COPY --from=build /usr/local/bin /usr/local/bin

ENTRYPOINT ["obsidian"]
