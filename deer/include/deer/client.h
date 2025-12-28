/**
 * @file client.h
 * @brief Lightweight client wiring for the Deer front-end.
 */

#ifndef DEERBOURNE_DEER_CLIENT_H
#define DEERBOURNE_DEER_CLIENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEER_DEFAULT_HOST "127.0.0.1"
#define DEER_DEFAULT_PORT 5555

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
    int socket_fd;           /**< Active socket descriptor; -1 when disconnected. */
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
 * @brief Attempt to connect to the configured Bourne endpoint.
 *
 * Performs a short-lived connection and issues a PING command, expecting a
 * PONG reply from the Bourne daemon.
 *
 * @param client Client configuration context.
 * @param message Optional message buffer for human-readable status.
 * @param message_len Length of the message buffer.
 * @return true when a TCP handshake succeeded and the daemon responded; false otherwise.
 */
bool deer_client_test_connection(DeerClient *client, char *message, size_t message_len);

/**
 * @brief Cleanly close any open socket associated with the client.
 *
 * This is safe to call even when no active connection exists.
 *
 * @param client Client configuration context.
 */
void deer_client_disconnect(DeerClient *client);

/**
 * @brief Submit user data to the Bourne service for storage or processing.
 *
 * Sends the payload to the Bourne service using the STORE command and captures
 * the immediate response.
 *
 * @param client Client configuration context.
 * @param payload User-provided content to send.
 * @param response Buffer to store the Bourne response.
 * @param response_len Length of the response buffer.
 * @return true when the payload was transmitted and a response received.
 */
bool deer_submit_data(const DeerClient *client, const char *payload, char *response, size_t response_len);

/**
 * @brief Perform a search query against the Bourne service.
 *
 * @param client Client configuration context.
 * @param query Query string to send to the service.
 * @param response Buffer to store the response content.
 * @param response_len Length of the response buffer.
 * @return true when the query executed and data was returned.
 */
bool deer_query_data(const DeerClient *client, const char *query, char *response, size_t response_len);

#endif // DEERBOURNE_DEER_CLIENT_H
