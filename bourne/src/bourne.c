/**
 * @file bourne.c
 * @brief Implementation for the DeerBourne back-end service (Bourne).
 */

#include "bourne.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BOURNE_DEFAULT_BIND_ADDRESS "127.0.0.1"
#define BOURNE_DEFAULT_PORT 5555
#define BOURNE_DEFAULT_BACKLOG 8

static void bourne_handle_client(int client_fd) {
    char buffer[256];
    ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        return;
    }

    buffer[received] = '\0';

    const char response[] = "ACK\n";
    send(client_fd, response, sizeof(response) - 1, 0);
}

static bool bourne_bind_and_listen(BourneServer *server) {
    if (!server) {
        return false;
    }

    server->listener_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->listener_fd < 0) {
        perror("bourne: socket");
        return false;
    }

    int enable = 1;
    if (setsockopt(server->listener_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("bourne: setsockopt");
        close(server->listener_fd);
        server->listener_fd = -1;
        return false;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server->config.port);
    if (inet_aton(server->config.bind_address, &addr.sin_addr) == 0) {
        fprintf(stderr, "bourne: invalid bind address %s\n", server->config.bind_address);
        close(server->listener_fd);
        server->listener_fd = -1;
        return false;
    }

    if (bind(server->listener_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bourne: bind");
        close(server->listener_fd);
        server->listener_fd = -1;
        return false;
    }

    if (listen(server->listener_fd, server->config.backlog) < 0) {
        perror("bourne: listen");
        close(server->listener_fd);
        server->listener_fd = -1;
        return false;
    }

    return true;
}

bool bourne_init(BourneServer *server, const BourneServerConfig *config) {
    if (!server) {
        return false;
    }

    BourneServerConfig defaults = {
        .bind_address = BOURNE_DEFAULT_BIND_ADDRESS,
        .port = BOURNE_DEFAULT_PORT,
        .backlog = BOURNE_DEFAULT_BACKLOG,
    };

    server->config.bind_address = (config && config->bind_address) ? config->bind_address : defaults.bind_address;
    server->config.port = (config && config->port != 0) ? config->port : defaults.port;
    server->config.backlog = (config && config->backlog > 0) ? config->backlog : defaults.backlog;
    server->listener_fd = -1;
    server->running = false;

    return bourne_bind_and_listen(server);
}

void bourne_run(BourneServer *server) {
    if (!server || server->listener_fd < 0) {
        return;
    }

    server->running = true;

    while (server->running) {
        int client_fd = accept(server->listener_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("bourne: accept");
            break;
        }

        bourne_handle_client(client_fd);
        close(client_fd);
    }
}

void bourne_shutdown(BourneServer *server) {
    if (!server) {
        return;
    }

    server->running = false;

    if (server->listener_fd >= 0) {
        close(server->listener_fd);
        server->listener_fd = -1;
    }
}
