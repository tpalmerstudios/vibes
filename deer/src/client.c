/**
 * @file client.c
 * @brief Lightweight client wiring for the Deer front-end.
 */

#define _POSIX_C_SOURCE 200809L

#include "deer/client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static bool deer_client_send_raw(const DeerClient *client, const char *request, char *response, size_t response_len,
                                 char *status, size_t status_len);

void deer_init(DeerClient *client, const DeerClientConfig *config) {
    if (!client) {
        return;
    }

    client->config.host = (config && config->host) ? config->host : DEER_DEFAULT_HOST;
    client->config.port = (config && config->port != 0) ? config->port : DEER_DEFAULT_PORT;
    client->socket_fd = -1;
}

bool deer_submit_data(const DeerClient *client, const char *payload, char *response, size_t response_len) {
    if (!client || !payload) {
        return false;
    }

    char request[640];
    snprintf(request, sizeof(request), "STORE %s\n", payload);
    return deer_client_send_raw(client, request, response, response_len, NULL, 0U);
}

bool deer_query_data(const DeerClient *client, const char *query, char *response, size_t response_len) {
    if (!client || !query) {
        return false;
    }

    char request[640];
    snprintf(request, sizeof(request), "SEARCH %s\n", query);
    return deer_client_send_raw(client, request, response, response_len, NULL, 0U);
}

static void deer_set_message(char *message, size_t message_len, const char *format, const char *detail) {
    if (!message || message_len == 0) {
        return;
    }

    if (format && detail) {
        snprintf(message, message_len, format, detail);
    } else if (format) {
        snprintf(message, message_len, "%s", format);
    } else {
        message[0] = '\0';
    }
}

static int deer_open_socket(const char *host, uint16_t port, char *message, size_t message_len) {
    if (!host || port == 0) {
        deer_set_message(message, message_len, "Invalid host or port", NULL);
        return -1;
    }

    char port_buffer[6];
    snprintf(port_buffer, sizeof(port_buffer), "%u", port);

    struct addrinfo hints = {0};
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    struct addrinfo *results = NULL;
    int rc = getaddrinfo(host, port_buffer, &hints, &results);
    if (rc != 0) {
        deer_set_message(message, message_len, "Address resolution failed: %s", gai_strerror(rc));
        return -1;
    }

    int fd = -1;
    for (struct addrinfo *entry = results; entry != NULL; entry = entry->ai_next) {
        fd = socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (fd < 0) {
            continue;
        }

        if (connect(fd, entry->ai_addr, entry->ai_addrlen) == 0) {
            break;
        }

        close(fd);
        fd = -1;
    }

    freeaddrinfo(results);

    if (fd < 0) {
        deer_set_message(message, message_len, "Connection failed: %s", strerror(errno));
    } else {
        deer_set_message(message, message_len, "Connected to Bourne at %s", host);
    }

    return fd;
}

static bool deer_send_and_receive(int fd, const char *request, char *response, size_t response_len, char *status,
                                  size_t status_len) {
    if (fd < 0 || !request) {
        deer_set_message(status, status_len, "Invalid socket or request", NULL);
        return false;
    }

    ssize_t sent = send(fd, request, strlen(request), 0);
    if (sent < 0) {
        deer_set_message(status, status_len, "Send failed: %s", strerror(errno));
        return false;
    }

    if (response && response_len > 0) {
        ssize_t received = recv(fd, response, response_len - 1U, 0);
        if (received < 0) {
            deer_set_message(status, status_len, "Receive failed: %s", strerror(errno));
            return false;
        }
        response[received] = '\0';
    }

    return true;
}

static bool deer_client_send_raw(const DeerClient *client, const char *request, char *response, size_t response_len,
                                 char *status, size_t status_len) {
    if (!client || !request) {
        deer_set_message(status, status_len, "Client not initialized", NULL);
        return false;
    }

    char local_status[128] = {0};
    int fd = deer_open_socket(client->config.host, client->config.port, local_status, sizeof(local_status));
    if (fd < 0) {
        deer_set_message(status, status_len, local_status, NULL);
        return false;
    }

    bool ok = deer_send_and_receive(fd, request, response, response_len, local_status, sizeof(local_status));
    close(fd);

    deer_set_message(status, status_len, local_status, NULL);
    return ok;
}

bool deer_client_test_connection(DeerClient *client, char *message, size_t message_len) {
    if (!client) {
        deer_set_message(message, message_len, "Client not initialized", NULL);
        return false;
    }

    const char *request = "PING\n";
    char response[64] = {0};
    bool ok = deer_client_send_raw(client, request, response, sizeof(response), message, message_len);
    if (ok && strncmp(response, "PONG", 4) != 0) {
        deer_set_message(message, message_len, "Unexpected response: %s", response);
        ok = false;
    }
    if (ok && message && message_len > 0) {
        deer_set_message(message, message_len, "Connected and responsive", NULL);
    }

    return ok;
}

void deer_client_disconnect(DeerClient *client) {
    if (!client) {
        return;
    }

    if (client->socket_fd >= 0) {
        close(client->socket_fd);
    }
    client->socket_fd = -1;
}
