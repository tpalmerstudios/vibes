/**
 * @file service.h
 * @brief Service lifecycle orchestration for Bourne.
 */

#ifndef DEERBOURNE_BOURNE_SERVICE_H
#define DEERBOURNE_BOURNE_SERVICE_H

#include <stdbool.h>
#include <stdatomic.h>

#include "bourne/config.h"
#include "bourne/threading.h"

/**
 * @brief Aggregated runtime state for the Bourne service.
 */
typedef struct {
    int listener_fd;           /**< Listening socket descriptor. */
    BourneServerConfig config; /**< Effective configuration. */
    BourneWorkerPool workers;  /**< Worker pool handling incoming clients. */
    _Atomic bool running;      /**< Indicates whether the service loop is active. */
} BourneServiceState;

/**
 * @brief Initialize the service state and prepare resources.
 *
 * @param state State instance to populate.
 * @param config Optional configuration overrides.
 * @return true when the service can proceed; false otherwise.
 */
bool bourne_service_init(BourneServiceState *state, const BourneServerConfig *config);

/**
 * @brief Start worker threads and begin accepting incoming connections.
 *
 * Starts the listener, installs signal handlers for SIGINT/SIGTERM, and spins
 * up the worker pool to handle socket accepts and protocol commands.
 *
 * @param state Initialized service state.
 * @return true when startup succeeded; false otherwise.
 */
bool bourne_service_start(BourneServiceState *state);

/**
 * @brief Block while the service loop continues to accept clients.
 *
 * This helper monitors the running flag and sleeps until shutdown is
 * requested. It is intended for use by the service executable to remain
 * resident without spinning.
 *
 * @param state Initialized and started service state.
 * @return true when the loop exited due to a deliberate stop request; false on
 *         invalid input.
 */
bool bourne_service_run(BourneServiceState *state);

/**
 * @brief Request a graceful shutdown, used by signals or protocol commands.
 *
 * This function is safe to call from any thread and will mark the service as no
 * longer running while closing the listener to wake worker threads.
 *
 * @param state Service state to stop.
 */
void bourne_service_request_stop(BourneServiceState *state);

/**
 * @brief Signal the service to stop and release all resources.
 *
 * @param state Service state to tear down.
 */
void bourne_service_stop(BourneServiceState *state);

#endif // DEERBOURNE_BOURNE_SERVICE_H
