/**
 * @file logging.h
 * @brief Simple logging helpers for the Bourne daemon.
 */

#ifndef DEERBOURNE_BOURNE_LOGGING_H
#define DEERBOURNE_BOURNE_LOGGING_H

#include <stdio.h>

/**
 * @brief Emit a verbose log line tagged for the Bourne daemon.
 *
 * @param fmt printf-style message.
 * @param ... Arguments forwarded to printf.
 */
#define BOURNE_LOG_VERBOSE(fmt, ...) fprintf(stderr, "[bourne][verbose] " fmt "\n", ##__VA_ARGS__)

/**
 * @brief Emit an error log line tagged for the Bourne daemon.
 *
 * @param fmt printf-style message.
 * @param ... Arguments forwarded to printf.
 */
#define BOURNE_LOG_ERROR(fmt, ...) fprintf(stderr, "[bourne][error] " fmt "\n", ##__VA_ARGS__)

#endif // DEERBOURNE_BOURNE_LOGGING_H
