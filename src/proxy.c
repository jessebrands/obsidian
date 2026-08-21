/*
 * proxy.c: minecraft client proxy
 *
 * This file implements a simple passthrough proxy, I use it to poke at the
 * protocol and to verify the veracity of my encoder and decoder.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <liburing.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "packet/buffer.h"

enum phase {
    PHASE_RECEIVE,
    PHASE_SEND,
};

struct relay {
    int from;
    int to;
    enum phase phase;
    struct pkt_buffer buffer;
};

static void
handle_receive(struct io_uring* io, struct relay* relay,
               struct io_uring_cqe* cqe) {
    if (cqe->res < 0) {
        fprintf(stderr, "error: receive failed: %s\n", strerror(-cqe->res));
        exit(EXIT_FAILURE);
    }

    /* check how many bytes we got */
    size_t bytes_in = (size_t) cqe->res;

    /* update buffer state */
    relay->buffer.cur += bytes_in;

    /* todo: decode packets here if we can */


    /* send this data to the next */
    struct io_uring_sqe* sqe = io_uring_get_sqe(io);
    relay->phase = PHASE_SEND;
    io_uring_prep_send(
        sqe,
        relay->to,
        &relay->buffer.data[relay->buffer.pos],
        relay->buffer.cur - relay->buffer.pos,
        0
    );
    io_uring_sqe_set_data(sqe, relay);

    /* done with this CQE */
    io_uring_cqe_seen(io, cqe);
}

static void
handle_send(struct io_uring* io, struct relay* relay,
            struct io_uring_cqe* cqe) {
    if (cqe->res < 0) {
        fprintf(stderr, "error: send failed: %s\n", strerror(-cqe->res));
        exit(EXIT_FAILURE);
    }

    /* check how many bytes we got */
    size_t const bytes_out = (size_t) cqe->res;

    /* update buffer state */
    relay->buffer.pos += bytes_out;
    relay->buffer.out_total += bytes_out;
    pkt_buffer_drop(&relay->buffer);

    struct io_uring_sqe* sqe = io_uring_get_sqe(io);
    if (relay->buffer.pos == relay->buffer.cur) {
        /* we're sent all bytes, so get ready to receive */
        relay->phase = PHASE_RECEIVE;
        io_uring_prep_recv(
            sqe,
            relay->from,
            &relay->buffer.data[relay->buffer.cur],
            relay->buffer.capacity - relay->buffer.cur,
            0
        );
    } else {
        /* didn't send all bytes yet, so continue sending */
        io_uring_prep_send(
            sqe,
            relay->to,
            &relay->buffer.data[relay->buffer.pos],
            relay->buffer.cur - relay->buffer.pos,
            0
        );
    }
    io_uring_sqe_set_data(sqe, relay);
    io_uring_cqe_seen(io, cqe);
}

static void
proxy(struct relay* client, struct relay* server) {
    printf("Initializing io_uring\n");
    struct io_uring io = {0};
    if (io_uring_queue_init(64, &io, 0) != 0) {
        perror("io_uring_queue_init");
        return;
    }

    /* stage the initial receive from the client */
    struct io_uring_sqe* sqe = io_uring_get_sqe(&io);
    client->phase = PHASE_RECEIVE;
    io_uring_prep_recv(sqe, client->from, client->buffer.data, client->buffer.capacity, 0);
    io_uring_sqe_set_data(sqe, client);

    /* and the initial receive from the server */
    sqe = io_uring_get_sqe(&io);
    server->phase = PHASE_RECEIVE;
    io_uring_prep_recv(sqe, server->from, server->buffer.data, server->buffer.capacity, 0);
    io_uring_sqe_set_data(sqe, server);

    /* i/o loop */
    while (true) {
        if (io_uring_submit(&io) < 0) {
            perror("io_uring_submit");
            break;
        }

        struct io_uring_cqe* cqe;
        if (io_uring_wait_cqe(&io, &cqe) < 0) {
            perror("io_uring_wait_cqe");
            break;
        }

        struct relay* relay = io_uring_cqe_get_data(cqe);
        if (relay->phase == PHASE_RECEIVE) {
            handle_receive(&io, relay, cqe);
        } else if (relay->phase == PHASE_SEND) {
            handle_send(&io, relay, cqe);
        }
    }

    io_uring_queue_exit(&io);
}

int main(int argc, char** argv) {
    setbuf(stdout, NULL);

    int status;
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    /* sit on port 25566 and wait for a connection */
    struct addrinfo* proxy_info;
    if ((status = getaddrinfo(NULL, "25566", &hints, &proxy_info)) != 0) {
        perror("failed to get address info");
        return EXIT_FAILURE;
    }

    int proxy_fd = socket(proxy_info->ai_family, proxy_info->ai_socktype, proxy_info->ai_protocol);
    if (proxy_fd == -1) {
        perror("failed to get a socket fd");
        return EXIT_FAILURE;
    }

    int const yes = 1;
    setsockopt(proxy_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    if (bind(proxy_fd, proxy_info->ai_addr, proxy_info->ai_addrlen) == -1) {
        perror("failed to bind");
        return EXIT_FAILURE;
    }

    if (listen(proxy_fd, 1) == -1) {
        perror("failed to listen");
        return EXIT_FAILURE;
    }

    while (true) {
        printf("Waiting for client connection\n");

        /* wait for the client to connect */
        struct sockaddr_storage client_addr;
        socklen_t client_len = sizeof client_addr;
        int client_fd = accept(proxy_fd, (struct sockaddr*) &client_addr, &client_len);
        if (client_fd == -1) {
            perror("failed to accept client connection");
            return EXIT_FAILURE;
        }

        printf("Accepted client connection\n");

        /* connect to the server on the client's behalf */
        hints = (struct addrinfo){0};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        struct addrinfo* server_info;
        if (getaddrinfo(NULL, "25565", &hints, &server_info) != 0) {
            perror("getaddrinfo");
            return EXIT_FAILURE;
        }

        int server_fd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
        if (server_fd == -1) {
            perror("socket");
            return EXIT_FAILURE;
        }

        if (connect(server_fd, server_info->ai_addr, server_info->ai_addrlen) == -1) {
            perror("connect");
            return EXIT_FAILURE;
        }

        struct relay client = {.from = client_fd, .to = server_fd};
        if (pkt_buffer_init(&client.buffer, 512) == NULL) {
            perror("reader_init");
            return EXIT_FAILURE;
        }

        struct relay server = {.from = server_fd, .to = client_fd};
        if (pkt_buffer_init(&server.buffer, 1024 * 16) == NULL) {
            perror("reader_init");
            return EXIT_FAILURE;
        }

        printf("Connected to server\n");
        proxy(&client, &server);

        /* we're done! */
        printf("Connections closed\n");
        pkt_buffer_end(&server.buffer);
        pkt_buffer_end(&client.buffer);
        close(server_fd);
        close(client_fd);
    }

    close(proxy_fd);
    freeaddrinfo(proxy_info);
    return 0;
}
