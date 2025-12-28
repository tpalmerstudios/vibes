/**
 * @file deer_app.c
 * @brief GTK4 front-end entry point for DeerBourne.
 */

#include "deer.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    DeerUiContext *context = (DeerUiContext *)user_data;
    GtkWidget *window = deer_ui_build_main_window(app, context);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    DeerUiContext context = {0};
    deer_init(&context.client, NULL);
    deer_ui_config_use_defaults(&context.ui_config);

    const char *config_path = (argc > 1) ? argv[1] : NULL;
    deer_load_ui_config(config_path, &context.ui_config);

    context.history = gtk_string_list_new(NULL);
    context.saved_items = gtk_string_list_new(NULL);
    context.search_buffer = gtk_entry_buffer_new(NULL, -1);
    context.data_entry_buffer = gtk_entry_buffer_new(NULL, -1);

    GtkApplication *app = gtk_application_new("com.deerbourne.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &context);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    g_clear_object(&context.provider);
    g_object_unref(context.history);
    g_object_unref(context.saved_items);
    g_object_unref(context.search_buffer);
    g_object_unref(context.data_entry_buffer);
    deer_free_ui_config(&context.ui_config);

    return status;
}
