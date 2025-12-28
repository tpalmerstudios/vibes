/**
 * @file service.c
 * @brief Service lifecycle orchestration for Bourne.
 */

#define _POSIX_C_SOURCE 200809L

#include "bourne/service.h"

#include "bourne/config.h"
#include "bourne/logging.h"
#include "bourne/network.h"
#include "bourne/query.h"
#include "bourne/storage.h"
#include "bourne/threading.h"

#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static volatile sig_atomic_t g_stop_requested = 0; /**< Tracks signal-driven shutdown. */

static void bourne_signal_handler(int signum) {
    (void)signum;
    g_stop_requested = 1;
}

static void bourne_install_signal_handlers(void) {
    struct sigaction action = {0};
    action.sa_handler = bourne_signal_handler;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}

static void bourne_prepare_config(BourneServiceState *state, const BourneServerConfig *config) {
    if (!state) {
        return;
    }

    bourne_config_use_defaults(&state->config);
    if (config) {
        state->config.bind_address = config->bind_address ? config->bind_address : state->config.bind_address;
        state->config.port = config->port ? config->port : state->config.port;
        state->config.backlog = config->backlog ? config->backlog : state->config.backlog;
        state->config.max_workers = config->max_workers ? config->max_workers : state->config.max_workers;
    }
}

static bool bourne_should_continue(const BourneServiceState *state) {
    return state && atomic_load(&state->running) && !g_stop_requested;
}

static void bourne_handle_client(int client_fd, BourneServiceState *state) {
    if (client_fd < 0 || !state) {
        return;
    }

    char buffer[256] = {0};
    ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        BOURNE_LOG_VERBOSE("connection closed before data received");
        return;
    }

    buffer[received] = '\0';

    BourneCommand command = {0};
    if (!bourne_query_parse(buffer, &command) || !bourne_query_validate(&command)) {
        const char response[] = "ERROR invalid command\n";
        send(client_fd, response, sizeof(response) - 1, 0);
        return;
    }

    switch (command.type) {
    case BOURNE_COMMAND_PING: {
        const char response[] = "PONG\n";
        send(client_fd, response, sizeof(response) - 1, 0);
        break;
    }
    case BOURNE_COMMAND_SEARCH: {
        char response[1024];
        size_t matches = bourne_storage_search(command.argument, response, sizeof(response));
        char header[64];
        snprintf(header, sizeof(header), "RESULT %zu\n", matches);
        send(client_fd, header, strlen(header), 0);
        send(client_fd, response, strlen(response), 0);
        break;
    }
    case BOURNE_COMMAND_STORE: {
        char status[128] = {0};
        if (bourne_storage_store(command.argument, status, sizeof(status))) {
            char response[196];
            snprintf(response, sizeof(response), "STORED %s\n", status);
            send(client_fd, response, strlen(response), 0);
        } else {
            char response[196];
            snprintf(response, sizeof(response), "ERROR %s\n", status[0] ? status : "store failed");
            send(client_fd, response, strlen(response), 0);
        }
        break;
    }
    case BOURNE_COMMAND_SHUTDOWN: {
        const char shutdown_response[] = "SHUTTING DOWN\n";
        send(client_fd, shutdown_response, sizeof(shutdown_response) - 1, 0);
        bourne_service_request_stop(state);
        break;
    }
    case BOURNE_COMMAND_INVALID:
    default: {
        const char response[] = "ERROR unsupported\n";
        send(client_fd, response, sizeof(response) - 1, 0);
        break;
    }
    }
}

static void *bourne_worker_accept_loop(void *context) {
    BourneServiceState *state = (BourneServiceState *)context;
    if (!state) {
        return NULL;
    }

    while (bourne_should_continue(state)) {
        int client_fd = bourne_network_accept(state->listener_fd);
        if (client_fd < 0) {
            if (!bourne_should_continue(state)) {
                break;
            }
            continue;
        }

        bourne_handle_client(client_fd, state);
        bourne_network_close(client_fd);
    }

    return NULL;
}

bool bourne_service_init(BourneServiceState *state, const BourneServerConfig *config) {
    if (!state) {
        return false;
    }

    memset(state, 0, sizeof(*state));
    state->listener_fd = -1;
    bourne_prepare_config(state, config);
    g_stop_requested = 0;

    if (!bourne_config_validate(&state->config)) {
        BOURNE_LOG_ERROR("configuration validation failed");
        return false;
    }

    if (!bourne_storage_init()) {
        BOURNE_LOG_ERROR("storage subsystem failed to initialize");
        return false;
    }

    state->listener_fd = bourne_network_listen(&state->config);
    if (state->listener_fd < 0) {
        BOURNE_LOG_ERROR("listener setup failed");
        bourne_storage_shutdown();
        return false;
    }

    return true;
}

bool bourne_service_start(BourneServiceState *state) {
    if (!state) {
        return false;
    }

    bourne_install_signal_handlers();
    atomic_store(&state->running, true);

    if (!bourne_workers_start(&state->workers, state->config.max_workers, bourne_worker_accept_loop, state)) {
        BOURNE_LOG_ERROR("worker pool start failed");
        bourne_service_stop(state);
        return false;
    }

    BOURNE_LOG_VERBOSE("service stub started on %s:%u with %zu workers",
                       state->config.bind_address,
                       state->config.port,
                       state->config.max_workers);

    return true;
}

bool bourne_service_run(BourneServiceState *state) {
    if (!state) {
        return false;
    }

    while (bourne_should_continue(state)) {
        sleep(1);
    }

    return true;
}

void bourne_service_stop(BourneServiceState *state) {
    if (!state) {
        return;
    }

    bourne_service_request_stop(state);
    bourne_workers_stop(&state->workers);
    bourne_network_close(state->listener_fd);
    bourne_storage_shutdown();
}

void bourne_service_request_stop(BourneServiceState *state) {
    if (!state) {
        return;
    }

    atomic_store(&state->running, false);
    if (state->listener_fd >= 0) {
        bourne_network_close(state->listener_fd);
        state->listener_fd = -1;
    }
}
