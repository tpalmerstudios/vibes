/**
 * @file deer_app.c
 * @brief GTK4 front-end entry point for DeerBourne.
 */

#include <gtk/gtk.h>

#include "deer.h"

typedef struct {
    DeerClient client; /**< Front-end client configuration for future network calls. */
} DeerUiContext;

static GtkWidget *build_main_window(GtkApplication *app, DeerUiContext *context) {
    (void)context;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "DeerBourne");
    gtk_window_set_default_size(GTK_WINDOW(window), 320, 200);

    GtkWidget *label = gtk_label_new("Hello from DeerBourne");
    gtk_window_set_child(GTK_WINDOW(window), label);

    return window;
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    DeerUiContext *context = (DeerUiContext *)user_data;
    GtkWidget *window = build_main_window(app, context);
    gtk_widget_show(window);
}

int main(int argc, char **argv) {
    DeerUiContext context = {0};
    deer_init(&context.client, NULL);

    GtkApplication *app = gtk_application_new("com.deerbourne.app", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &context);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
