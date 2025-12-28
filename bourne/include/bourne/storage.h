/**
 * @file storage.h
 * @brief Persistent storage surface for JSON payloads.
 */

#ifndef DEERBOURNE_BOURNE_STORAGE_H
#define DEERBOURNE_BOURNE_STORAGE_H

#include <stdbool.h>

/**
 * @brief Placeholder for the storage engine initialization.
 *
 * The implementation is intentionally left as a stub to be expanded with
 * JSON serialization and disk persistence.
 *
 * @return true when initialization succeeded; false otherwise.
 */
bool bourne_storage_init(void);

/**
 * @brief Tear down storage internals.
 */
void bourne_storage_shutdown(void);

#endif // DEERBOURNE_BOURNE_STORAGE_H
