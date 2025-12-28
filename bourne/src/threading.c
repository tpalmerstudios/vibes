/**
 * @file threading.c
 * @brief Threading facilities for the Bourne daemon.
 */

#include "bourne/threading.h"

#include "bourne/logging.h"

#include <stdlib.h>

static void *bourne_worker_stub(void *context) {
    BOURNE_LOG_VERBOSE("worker thread stub started (context=%p)", context);
    return NULL;
}

bool bourne_workers_start(BourneWorkerPool *pool, size_t count, void *(*routine)(void *), void *context) {
    if (!pool || count == 0) {
        return false;
    }

    pool->threads = (pthread_t *)calloc(count, sizeof(pthread_t));
    if (!pool->threads) {
        return false;
    }

    pool->count = count;
    pool->running = true;

    void *(*worker_routine)(void *) = routine ? routine : bourne_worker_stub;

    for (size_t i = 0; i < count; ++i) {
        if (pthread_create(&pool->threads[i], NULL, worker_routine, context) != 0) {
            BOURNE_LOG_ERROR("failed to start worker %zu", i);
            pool->count = i;
            pool->running = false;
            bourne_workers_stop(pool);
            return false;
        }
    }

    return true;
}

void bourne_workers_stop(BourneWorkerPool *pool) {
    if (!pool || !pool->threads) {
        return;
    }

    pool->running = false;

    for (size_t i = 0; i < pool->count; ++i) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);
    pool->threads = NULL;
    pool->count = 0;
}
