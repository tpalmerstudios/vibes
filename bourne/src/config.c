/**
 * @file config.c
 * @brief Configuration helpers for the Bourne back-end service.
 */

#include "bourne/config.h"

#include <limits.h>

#define BOURNE_DEFAULT_BIND_ADDRESS "127.0.0.1"
#define BOURNE_DEFAULT_PORT 5555
#define BOURNE_DEFAULT_BACKLOG 8
#define BOURNE_DEFAULT_WORKERS 4

void bourne_config_use_defaults(BourneServerConfig *config) {
    if (!config) {
        return;
    }

    config->bind_address = BOURNE_DEFAULT_BIND_ADDRESS;
    config->port = BOURNE_DEFAULT_PORT;
    config->backlog = BOURNE_DEFAULT_BACKLOG;
    config->max_workers = BOURNE_DEFAULT_WORKERS;
}

bool bourne_config_validate(const BourneServerConfig *config) {
    if (!config) {
        return false;
    }

    if (!config->bind_address || config->bind_address[0] == '\0') {
        return false;
    }

    if (config->port == 0 || config->port > USHRT_MAX) {
        return false;
    }

    if (config->backlog <= 0) {
        return false;
    }

    if (config->max_workers == 0) {
        return false;
    }

    return true;
}
