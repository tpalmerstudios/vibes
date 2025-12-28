/**
 * @file deer_app.c
 * @brief GTK4 front-end entry point for DeerBourne.
 */

#include "deer.h"

#include <glib.h>

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
    context.default_layout = context.ui_config.layout;

    deer_sample_data_load(NULL, &context.sample_records);

    context.history = gtk_string_list_new(NULL);
    context.saved_items = gtk_string_list_new(NULL);
    context.sample_detail_buffer = gtk_text_buffer_new(NULL);
    context.relationships = gtk_string_list_new((const char *const[]){"Select a record to view relationships", NULL});
    context.search_buffer = gtk_entry_buffer_new(NULL, -1);
    context.data_entry_buffer = gtk_entry_buffer_new(NULL, -1);

    guint sample_count = context.sample_records ? context.sample_records->len : 0U;
    char **sample_strings = g_new0(char *, sample_count + 1U);
    for (guint i = 0; i < sample_count; ++i) {
        DeerSampleRecord *record = g_ptr_array_index(context.sample_records, i);
        const char *category = record->category ? record->category : "entry";
        sample_strings[i] = g_strdup_printf("[%s] %s", category, record->summary ? record->summary : record->title);
    }
    context.sample_items = gtk_string_list_new((const char *const *)sample_strings);
    g_strfreev(sample_strings);
    context.sample_selection = gtk_single_selection_new(G_LIST_MODEL(context.sample_items));

    if (sample_count > 0U) {
        gtk_single_selection_set_selected(context.sample_selection, 0);
        DeerSampleRecord *first = g_ptr_array_index(context.sample_records, 0);
        gtk_text_buffer_set_text(context.sample_detail_buffer, first->details ? first->details : "No details available.", -1);

        gtk_string_list_splice(context.relationships, 0, gtk_string_list_get_n_items(context.relationships), NULL);
        if (first->relationships && first->relationships->len > 0U) {
            for (guint i = 0; i < first->relationships->len; ++i) {
                const char *relation = g_ptr_array_index(first->relationships, i);
                gtk_string_list_append(context.relationships, relation ? relation : "Related entry");
            }
        } else {
            gtk_string_list_append(context.relationships, "No relationship data yet");
        }
    } else {
        gtk_text_buffer_set_text(context.sample_detail_buffer, "No sample data available yet.", -1);
    }

    GtkApplication *app = gtk_application_new("com.deerbourne.app", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), &context);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);
    g_clear_object(&context.provider);
    g_object_unref(context.history);
    g_object_unref(context.saved_items);
    g_clear_object(&context.sample_selection);
    g_clear_object(&context.sample_items);
    g_clear_object(&context.relationships);
    g_clear_object(&context.sample_detail_buffer);
    g_object_unref(context.search_buffer);
    g_object_unref(context.data_entry_buffer);
    deer_sample_data_free(context.sample_records);
    deer_free_ui_config(&context.ui_config);

    return status;
}
