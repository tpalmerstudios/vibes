/**
 * @file network.c
 * @brief Socket utilities for the Bourne daemon.
 */

#include "bourne/network.h"

#include "bourne/logging.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int bourne_network_listen(const BourneServerConfig *config) {
    if (!config) {
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        BOURNE_LOG_ERROR("socket creation failed: %s", strerror(errno));
        return -1;
    }

    int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        BOURNE_LOG_ERROR("setsockopt(SO_REUSEADDR) failed: %s", strerror(errno));
        close(fd);
        return -1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config->port);
    if (inet_pton(AF_INET, config->bind_address, &addr.sin_addr) != 1) {
        BOURNE_LOG_ERROR("invalid bind address '%s'", config->bind_address);
        close(fd);
        return -1;
    }

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        BOURNE_LOG_ERROR("bind failed: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (listen(fd, config->backlog) < 0) {
        BOURNE_LOG_ERROR("listen failed: %s", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

int bourne_network_accept(int listener_fd) {
    if (listener_fd < 0) {
        return -1;
    }

    int client_fd = accept(listener_fd, NULL, NULL);
    if (client_fd < 0) {
        BOURNE_LOG_VERBOSE("connection attempt failed: %s", strerror(errno));
        return -1;
    }

    return client_fd;
}

void bourne_network_close(int fd) {
    if (fd < 0) {
        return;
    }

    close(fd);
}
