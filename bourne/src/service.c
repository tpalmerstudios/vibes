/**
 * @file service.c
 * @brief Service lifecycle orchestration for Bourne.
 */

#include "bourne/service.h"

#include "bourne/config.h"
#include "bourne/logging.h"
#include "bourne/network.h"
#include "bourne/storage.h"
#include "bourne/threading.h"

#include <string.h>

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

bool bourne_service_init(BourneServiceState *state, const BourneServerConfig *config) {
    if (!state) {
        return false;
    }

    memset(state, 0, sizeof(*state));
    state->listener_fd = -1;
    bourne_prepare_config(state, config);

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

    state->running = true;

    if (!bourne_workers_start(&state->workers, state->config.max_workers, NULL, state)) {
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

void bourne_service_stop(BourneServiceState *state) {
    if (!state) {
        return;
    }

    state->running = false;
    bourne_workers_stop(&state->workers);
    bourne_network_close(state->listener_fd);
    bourne_storage_shutdown();
}
