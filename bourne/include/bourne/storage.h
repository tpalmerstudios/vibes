/**
 * @file storage.h
 * @brief Persistent storage surface for JSON payloads.
 */

#ifndef DEERBOURNE_BOURNE_STORAGE_H
#define DEERBOURNE_BOURNE_STORAGE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Maximum number of records maintained by the in-memory store.
 */
#define BOURNE_MAX_RECORDS 64U

/**
 * @brief Compact representation of a stored record for search responses.
 */
typedef struct {
    char title[128];   /**< Title or primary label for the record. */
    char summary[256]; /**< Short description of the record. */
    char raw[768];     /**< Raw JSON snippet retained for clients. */
} BourneRecord;

/**
 * @brief Placeholder for the storage engine initialization.
 *
 * Loads the default dataset from disk when present and falls back to a
 * compiled-in sample set to guarantee non-empty search results. Future
 * iterations can replace this with a full persistence layer.
 *
 * @return true when initialization succeeded; false otherwise.
 */
bool bourne_storage_init(void);

/**
 * @brief Tear down storage internals.
 */
void bourne_storage_shutdown(void);

/**
 * @brief Attempt to load records from a JSON file on disk.
 *
 * @param path Path to a JSON file formatted similarly to sample_records.json.
 * @return true when at least one record was loaded; false otherwise.
 */
bool bourne_storage_load(const char *path);

/**
 * @brief Retrieve the number of indexed records.
 *
 * @return The count of records currently cached in memory.
 */
size_t bourne_storage_record_count(void);

/**
 * @brief Execute a case-insensitive substring search across cached records.
 *
 * The response buffer is populated with human-readable lines formatted as
 * "title — summary" for each matching record. When no matches are found the
 * buffer contains a short explanation string.
 *
 * @param query User-provided query string.
 * @param buffer Response buffer to populate.
 * @param buffer_len Length of the response buffer.
 * @return Number of matches written to the response buffer.
 */
size_t bourne_storage_search(const char *query, char *buffer, size_t buffer_len);

/**
 * @brief Store a raw payload into the in-memory record list.
 *
 * The payload is preserved as-is in the @ref BourneRecord::raw field, while the
 * title and summary are extracted from simple JSON members when present. When
 * the payload lacks these members, defaults are synthesized to keep responses
 * readable. The function returns false when storage is full or the payload is
 * empty.
 *
 * @param payload Raw user-supplied text, ideally JSON.
 * @param message Optional status buffer describing the result.
 * @param message_len Length of the status buffer.
 * @return true when the payload was indexed; false otherwise.
 */
bool bourne_storage_store(const char *payload, char *message, size_t message_len);

#endif // DEERBOURNE_BOURNE_STORAGE_H
