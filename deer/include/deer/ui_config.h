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
 * @brief UI layout toggles that allow the user to customize visible regions.
 */
typedef struct {
    bool show_search;        /**< Show or hide the main search controls. */
    bool show_history;       /**< Show or hide the history view. */
    bool show_saved_items;   /**< Show or hide saved items. */
    bool show_data_entry;    /**< Show or hide the data entry area. */
    bool show_theme_controls;/**< Show or hide theme color controls. */
    bool show_connection;    /**< Show or hide the connectivity probe. */
    bool show_sample_browser;/**< Show or hide the structured data explorer. */
} DeerUiLayoutOptions;

/**
 * @brief UI configuration applied to the GTK front-end.
 */
typedef struct {
    char *theme_name;            /**< Human-readable theme label. */
    DeerColorConfig colors;      /**< Theming palette. */
    DeerUiLayoutOptions layout;  /**< Layout preferences from config or runtime toggles. */
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
