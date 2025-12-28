/**
 * @file deer_app.c
 * @brief GTK4 front-end entry point for DeerBourne.
 */

#include <gtk/gtk.h>

#include "deer.h"

static void on_activate(GtkApplication *app, gpointer user_data) {
    (void)user_data;

    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "DeerBourne");
    gtk_window_set_default_size(GTK_WINDOW(window), 320, 200);

    GtkWidget *label = gtk_label_new("Hello from DeerBourne");
    gtk_window_set_child(GTK_WINDOW(window), label);

    gtk_widget_show(window);
}

int main(int argc, char **argv) {
    deer_init();

    GtkApplication *app = gtk_application_new("com.deerbourne.app", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
