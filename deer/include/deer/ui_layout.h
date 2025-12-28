/**
 * @file ui_layout.h
 * @brief GTK layout builders for Deer.
 */

#ifndef DEERBOURNE_DEER_UI_LAYOUT_H
#define DEERBOURNE_DEER_UI_LAYOUT_H

#include <gtk/gtk.h>

#include "deer/client.h"
#include "deer/sample_data.h"
#include "deer/ui_config.h"

/**
 * @brief State shared across the Deer GTK application.
 */
typedef struct {
    DeerClient client;      /**< Front-end client configuration for future network calls. */
    DeerUiConfig ui_config; /**< Active UI configuration (theme + colors). */
    DeerUiLayoutOptions default_layout; /**< Layout defaults captured at startup. */
    GPtrArray *sample_records; /**< Parsed showcase dataset. */
    GtkCssProvider *provider;
    GtkStringList *history;
    GtkStringList *saved_items;
    GtkStringList *sample_items;
    GtkStringList *relationships; /**< Live view of relationships for the selected record. */
    GtkSingleSelection *sample_selection;
    GtkEntryBuffer *search_buffer;
    GtkEntryBuffer *data_entry_buffer;
    GtkTextBuffer *sample_detail_buffer;
    GtkWidget *connection_status_label; /**< Displays connectivity probe results. */
    GtkApplication *app;                /**< Owning GTK application (for actions). */
    GtkWidget *search_area;             /**< Section reference for toggling visibility. */
    GtkWidget *history_view;
    GtkWidget *saved_view;
    GtkWidget *data_entry_area;
    GtkWidget *color_controls;
    GtkWidget *connection_area;
    GtkWidget *sample_area;
    GtkWidget *relationship_view;
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
