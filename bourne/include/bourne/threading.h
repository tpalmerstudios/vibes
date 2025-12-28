/**
 * @file threading.h
 * @brief Threading facilities for the Bourne daemon.
 */

#ifndef DEERBOURNE_BOURNE_THREADING_H
#define DEERBOURNE_BOURNE_THREADING_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Simple worker pool descriptor.
 */
typedef struct {
    pthread_t *threads; /**< Array of worker thread handles. */
    size_t count;       /**< Number of active worker threads. */
    bool running;       /**< Indicates whether workers should continue looping. */
} BourneWorkerPool;

/**
 * @brief Start a lightweight worker pool for handling front-end connections.
 *
 * @param pool Pool instance to populate.
 * @param count Number of workers to spawn.
 * @param routine Worker routine to execute per thread.
 * @param context Shared context passed to each worker.
 * @return true on success; false otherwise.
 */
bool bourne_workers_start(BourneWorkerPool *pool, size_t count, void *(*routine)(void *), void *context);

/**
 * @brief Signal workers to stop and release resources.
 *
 * @param pool Pool to tear down.
 */
void bourne_workers_stop(BourneWorkerPool *pool);

#endif // DEERBOURNE_BOURNE_THREADING_H
