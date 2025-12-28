/**
 * @file query.h
 * @brief Lightweight parser and validator for Bourne protocol commands.
 */

#ifndef DEERBOURNE_BOURNE_QUERY_H
#define DEERBOURNE_BOURNE_QUERY_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Supported command types.
 */
typedef enum {
    BOURNE_COMMAND_INVALID = 0, /**< Parsing failed or unknown command. */
    BOURNE_COMMAND_PING,        /**< Connectivity probe. */
    BOURNE_COMMAND_SEARCH,      /**< Search across stored records. */
    BOURNE_COMMAND_STORE,       /**< Persist a JSON payload. */
    BOURNE_COMMAND_SHUTDOWN     /**< Request graceful shutdown. */
} BourneCommandType;

/**
 * @brief Parsed command container.
 */
typedef struct {
    BourneCommandType type; /**< Command verb. */
    char argument[512];     /**< Optional argument such as a query string. */
} BourneCommand;

/**
 * @brief Parse an incoming raw request into a BourneCommand structure.
 *
 * @param raw Incoming request buffer.
 * @param command Output structure populated with the parsed result.
 * @return true when parsing succeeded; false otherwise.
 */
bool bourne_query_parse(const char *raw, BourneCommand *command);

/**
 * @brief Validate a parsed command to ensure required arguments are present.
 *
 * @param command Parsed command to validate.
 * @return true when the command is well-formed and actionable.
 */
bool bourne_query_validate(const BourneCommand *command);

#endif // DEERBOURNE_BOURNE_QUERY_H
