/**
 * @file service.h
 * @brief Service lifecycle orchestration for Bourne.
 */

#ifndef DEERBOURNE_BOURNE_SERVICE_H
#define DEERBOURNE_BOURNE_SERVICE_H

#include <stdbool.h>

#include "bourne/config.h"
#include "bourne/threading.h"

/**
 * @brief Aggregated runtime state for the Bourne service.
 */
typedef struct {
    int listener_fd;           /**< Listening socket descriptor. */
    BourneServerConfig config; /**< Effective configuration. */
    BourneWorkerPool workers;  /**< Worker pool handling incoming clients. */
    bool running;              /**< Indicates whether the service loop is active. */
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
 * This is currently a stubbed entry point that wires together the
 * configuration, listener, and worker pool without implementing the
 * full dispatch loop. It is intentionally structured for future expansion.
 *
 * @param state Initialized service state.
 * @return true when startup succeeded; false otherwise.
 */
bool bourne_service_start(BourneServiceState *state);

/**
 * @brief Signal the service to stop and release all resources.
 *
 * @param state Service state to tear down.
 */
void bourne_service_stop(BourneServiceState *state);

#endif // DEERBOURNE_BOURNE_SERVICE_H
