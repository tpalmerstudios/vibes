/**
 * @file deer.h
 * @brief Interfaces for the DeerBourne front-end (Deer) communication layer.
 */

#ifndef DEERBOURNE_DEER_H
#define DEERBOURNE_DEER_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Configuration for Deer client connections.
 */
typedef struct {
    const char *host; /**< Hostname or IP to reach the Bourne service. */
    uint16_t port;    /**< TCP port for the Bourne service. */
} DeerClientConfig;

/**
 * @brief Color palette for the front-end theme.
 */
typedef struct {
    char *background; /**< Background color (CSS format). */
    char *foreground; /**< Foreground/text color (CSS format). */
    char *accent;     /**< Accent color for highlights (CSS format). */
} DeerColorConfig;

/**
 * @brief UI configuration applied to the GTK front-end.
 */
typedef struct {
    char *theme_name;       /**< Human-readable theme label. */
    DeerColorConfig colors; /**< Theming palette. */
} DeerUiConfig;

/**
 * @brief Lightweight client state used by the front-end.
 */
typedef struct {
    DeerClientConfig config; /**< Target connection configuration. */
} DeerClient;

/**
 * @brief Initialize the front-end communication layer.
 *
 * @param client Client instance to configure.
 * @param config Configuration values; defaults are applied when NULL.
 */
void deer_init(DeerClient *client, const DeerClientConfig *config);

/**
 * @brief Apply default UI configuration values.
 *
 * @param config Configuration structure to populate.
 */
void deer_ui_config_use_defaults(DeerUiConfig *config);

/**
 * @brief Load UI configuration values from an ini-style file.
 *
 * @param path Path to the configuration file.
 * @param out_config Configuration structure to populate.
 * @return true when the file was parsed successfully; false otherwise.
 */
bool deer_load_ui_config(const char *path, DeerUiConfig *out_config);

/**
 * @brief Release dynamically allocated fields in a UI configuration.
 *
 * @param config Configuration previously filled by defaults or parsing.
 */
void deer_free_ui_config(DeerUiConfig *config);

/**
 * @brief Submit user data to the Bourne service for storage or processing.
 *
 * This is a stub meant to be expanded with real networking in future iterations.
 *
 * @param client Client configuration context.
 * @param payload User-provided content to send.
 */
void deer_submit_data(const DeerClient *client, const char *payload);

/**
 * @brief Perform a query against the Bourne service and return a placeholder response.
 *
 * @param client Client configuration context.
 * @param query Query string to send to the service.
 * @return A static string representing the placeholder result.
 */
const char *deer_query_data(const DeerClient *client, const char *query);

#endif // DEERBOURNE_DEER_H
