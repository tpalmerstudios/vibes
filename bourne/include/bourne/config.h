/**
 * @file config.h
 * @brief Configuration helpers for the Bourne back-end service.
 */

#ifndef DEERBOURNE_BOURNE_CONFIG_H
#define DEERBOURNE_BOURNE_CONFIG_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Tunable server configuration values for socket handling and workers.
 */
typedef struct {
    const char *bind_address; /**< IPv4/IPv6 address to bind (e.g., 127.0.0.1). */
    uint16_t port;            /**< TCP port to listen on. */
    int backlog;              /**< Accept queue depth. */
    size_t max_workers;       /**< Number of worker threads to spawn. */
} BourneServerConfig;

/**
 * @brief Populate a configuration structure with the project defaults.
 *
 * @param config Configuration instance to initialize.
 */
void bourne_config_use_defaults(BourneServerConfig *config);

/**
 * @brief Validate that the configuration values are usable for the daemon.
 *
 * @param config Configuration to verify.
 * @return true when all fields appear valid; false otherwise.
 */
bool bourne_config_validate(const BourneServerConfig *config);

#endif // DEERBOURNE_BOURNE_CONFIG_H
