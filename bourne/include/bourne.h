/**
 * @file bourne.h
 * @brief Interfaces for the DeerBourne back-end service (Bourne).
 */

#ifndef DEERBOURNE_BOURNE_H
#define DEERBOURNE_BOURNE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configuration for the Bourne TCP server.
 */
typedef struct {
    const char *bind_address; /**< IPv4 address to bind (e.g., 127.0.0.1). */
    uint16_t port;            /**< TCP port to listen on. */
    int backlog;              /**< Accept queue depth. */
} BourneServerConfig;

/**
 * @brief Runtime state for the Bourne TCP server.
 */
typedef struct {
    int listener_fd;              /**< File descriptor for the listening socket. */
    BourneServerConfig config;    /**< Effective configuration. */
    bool running;                 /**< Indicates if the main loop should continue. */
} BourneServer;

/**
 * @brief Initialize the back-end service state and TCP listener.
 *
 * @param server Server instance to initialize.
 * @param config Optional configuration; defaults are used when NULL.
 * @return true on success, false otherwise.
 */
bool bourne_init(BourneServer *server, const BourneServerConfig *config);

/**
 * @brief Run the Bourne service main control loop.
 *
 * This loop accepts incoming TCP connections on the configured local socket
 * and handles each client sequentially. It runs until stopped via
 * bourne_shutdown().
 *
 * @param server Initialized server instance.
 */
void bourne_run(BourneServer *server);

/**
 * @brief Stop the main loop and release server resources.
 *
 * @param server Server instance to shutdown.
 */
void bourne_shutdown(BourneServer *server);

#endif // DEERBOURNE_BOURNE_H
