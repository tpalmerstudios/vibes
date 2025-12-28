/**
 * @file ui_layout.h
 * @brief GTK layout builders for Deer.
 */

#ifndef DEERBOURNE_DEER_UI_LAYOUT_H
#define DEERBOURNE_DEER_UI_LAYOUT_H

#include <gtk/gtk.h>

#include "deer/client.h"
#include "deer/ui_config.h"

/**
 * @brief State shared across the Deer GTK application.
 */
typedef struct {
    DeerClient client;      /**< Front-end client configuration for future network calls. */
    DeerUiConfig ui_config; /**< Active UI configuration (theme + colors). */
    GtkCssProvider *provider;
    GtkStringList *history;
    GtkStringList *saved_items;
    GtkEntryBuffer *search_buffer;
    GtkEntryBuffer *data_entry_buffer;
} DeerUiContext;

/**
 * @brief Build the GTK window using the configured layout.
 *
 * @param app Owning GTK application.
 * @param context UI context containing buffers and configuration.
 * @return Newly constructed widget hierarchy.
 */
GtkWidget *deer_ui_build_main_window(GtkApplication *app, DeerUiContext *context);

#endif // DEERBOURNE_DEER_UI_LAYOUT_H
