/**
 * @file ui_config.h
 * @brief UI configuration structures and helpers for Deer.
 */

#ifndef DEERBOURNE_DEER_UI_CONFIG_H
#define DEERBOURNE_DEER_UI_CONFIG_H

#include <stdbool.h>

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

#endif // DEERBOURNE_DEER_UI_CONFIG_H
