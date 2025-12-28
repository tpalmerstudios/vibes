/**
 * @file client.h
 * @brief Lightweight client wiring for the Deer front-end.
 */

#ifndef DEERBOURNE_DEER_CLIENT_H
#define DEERBOURNE_DEER_CLIENT_H

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
 * This function currently records default or user-provided configuration
 * without opening any sockets. It is intended to be extended once the
 * networking protocol is finalized.
 *
 * @param client Client instance to configure.
 * @param config Configuration values; defaults are applied when NULL.
 */
void deer_init(DeerClient *client, const DeerClientConfig *config);

/**
 * @brief Submit user data to the Bourne service for storage or processing.
 *
 * This is a stub meant to be expanded with real networking in future iterations.
 *
 * @param client Client configuration context.
 * @param payload User-provided content to send.
 */
void deer_submit_data(const DeerClient *client, const char *payload);

/**
 * @brief Perform a query against the Bourne service and return a placeholder response.
 *
 * @param client Client configuration context.
 * @param query Query string to send to the service.
 * @return A static string representing the placeholder result.
 */
const char *deer_query_data(const DeerClient *client, const char *query);

#endif // DEERBOURNE_DEER_CLIENT_H
