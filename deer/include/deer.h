/**
 * @file deer.h
 * @brief Interfaces for the DeerBourne front-end (Deer) communication layer.
 */

#ifndef DEERBOURNE_DEER_H
#define DEERBOURNE_DEER_H

#include <stdint.h>

/**
 * @brief Configuration for Deer client connections.
 */
typedef struct {
    const char *host; /**< Hostname or IP to reach the Bourne service. */
    uint16_t port;    /**< TCP port for the Bourne service. */
} DeerClientConfig;

/**
 * @brief Lightweight client state used by the front-end.
 */
typedef struct {
    DeerClientConfig config; /**< Target connection configuration. */
} DeerClient;

/**
 * @brief Initialize the front-end communication layer.
 *
 * @param client Client instance to configure.
 * @param config Configuration values; defaults are applied when NULL.
 */
void deer_init(DeerClient *client, const DeerClientConfig *config);

#endif // DEERBOURNE_DEER_H
