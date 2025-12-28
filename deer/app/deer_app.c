/**
 * @file deer_app.c
 * @brief GTK4 front-end entry point for DeerBourne.
 */

#include <gtk/gtk.h>

#include "deer.h"

typedef struct {
    DeerClient client;      /**< Front-end client configuration for future network calls. */
    DeerUiConfig ui_config; /**< Active UI configuration (theme + colors). */
    GtkCssProvider *provider;
    GtkStringList *history;
    GtkStringList *saved_items;
    GtkEntryBuffer *search_buffer;
    GtkEntryBuffer *data_entry_buffer;
} DeerUiContext;

static void list_item_setup(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer user_data);
static void list_item_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer user_data);

static void apply_theme(GtkWidget *window, DeerUiContext *context) {
    if (!window || !context) {
        return;
    }

    if (!context->provider) {
        context->provider = gtk_css_provider_new();
    }

    gchar *css = g_strdup_printf(
        "* {"
        "  background-color: %s;"
        "  color: %s;"
        "}"
        "button {"
        "  background: linear-gradient(135deg, %s, %s);"
        "  color: %s;"
        "  border-radius: 8px;"
        "  padding: 6px 10px;"
        "}"
        "entry {"
        "  background-color: shade(%s, 1.1);"
        "  color: %s;"
        "  border-radius: 6px;"
        "  padding: 4px 6px;"
        "}",
        context->ui_config.colors.background,
        context->ui_config.colors.foreground,
        context->ui_config.colors.accent,
        context->ui_config.colors.background,
        context->ui_config.colors.foreground,
        context->ui_config.colors.background,
        context->ui_config.colors.foreground);

    gtk_css_provider_load_from_data(context->provider, css, -1);
    g_free(css);

    gtk_style_context_add_provider_for_display(gtk_widget_get_display(window),
                                               GTK_STYLE_PROVIDER(context->provider),
                                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void list_item_setup(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer user_data) {
    (void)factory;
    (void)user_data;
    GtkWidget *label = gtk_label_new(NULL);
    gtk_list_item_set_child(list_item, label);
}

static void list_item_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer user_data) {
    (void)factory;
    (void)user_data;
    GtkWidget *label = gtk_list_item_get_child(list_item);
    GtkStringObject *string_object = GTK_STRING_OBJECT(gtk_list_item_get_item(list_item));
    const char *item = gtk_string_object_get_string(string_object);
    gtk_label_set_text(GTK_LABEL(label), item);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0f);
}

static void on_history_append(DeerUiContext *context, const char *text) {
    if (!context || !text) {
        return;
    }
    gtk_string_list_append(context->history, text);
}

static void on_saved_append(DeerUiContext *context, const char *text) {
    if (!context || !text) {
        return;
    }
    gtk_string_list_append(context->saved_items, text);
}

static void on_search_activate(GtkButton *button, gpointer user_data) {
    (void)button;
    DeerUiContext *context = (DeerUiContext *)user_data;
    const char *query = gtk_entry_buffer_get_text(context->search_buffer);
    on_history_append(context, query);
    const char *result = deer_query_data(&context->client, query);
    on_saved_append(context, result);
}

static void on_data_submit(GtkButton *button, gpointer user_data) {
    (void)button;
    DeerUiContext *context = (DeerUiContext *)user_data;
    const char *payload = gtk_entry_buffer_get_text(context->data_entry_buffer);
    deer_submit_data(&context->client, payload);
    on_saved_append(context, payload);
}

static GtkWidget *build_list_view(GtkStringList *model, const char *title) {
    GtkWidget *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(list_item_setup), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(list_item_bind), NULL);

    GtkListView *list_view = GTK_LIST_VIEW(gtk_list_view_new(
        GTK_SELECTION_MODEL(gtk_single_selection_new(G_LIST_MODEL(model))), GTK_LIST_ITEM_FACTORY(factory)));

    GtkWidget *scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), GTK_WIDGET(list_view));
    gtk_widget_set_vexpand(scroller, TRUE);

    GtkWidget *frame = gtk_frame_new(title);
    gtk_frame_set_child(GTK_FRAME(frame), scroller);

    return frame;
}

static void update_color_from_button(GtkColorDialogButton *button, GParamSpec *pspec, gpointer user_data) {
    (void)pspec;
    DeerUiContext *context = (DeerUiContext *)user_data;
    GdkRGBA color;
    gtk_color_dialog_button_get_rgba(button, &color);
    gchar *color_string = gdk_rgba_to_string(&color);

    const char *widget_name = gtk_buildable_get_name(GTK_BUILDABLE(button));
    if (g_strcmp0(widget_name, "background-button") == 0) {
        g_free(context->ui_config.colors.background);
        context->ui_config.colors.background = color_string;
    } else if (g_strcmp0(widget_name, "foreground-button") == 0) {
        g_free(context->ui_config.colors.foreground);
        context->ui_config.colors.foreground = color_string;
    } else {
        g_free(context->ui_config.colors.accent);
        context->ui_config.colors.accent = color_string;
    }

    apply_theme(gtk_widget_get_root(GTK_WIDGET(button)), context);
}

static GtkWidget *build_color_controls(DeerUiContext *context) {
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 6);

    GtkWidget *bg_label = gtk_label_new("Background");
    GtkWidget *fg_label = gtk_label_new("Foreground");
    GtkWidget *accent_label = gtk_label_new("Accent");

    GtkWidget *bg_button = gtk_color_dialog_button_new(NULL);
    gtk_buildable_set_name(GTK_BUILDABLE(bg_button), "background-button");
    GtkWidget *fg_button = gtk_color_dialog_button_new(NULL);
    gtk_buildable_set_name(GTK_BUILDABLE(fg_button), "foreground-button");
    GtkWidget *accent_button = gtk_color_dialog_button_new(NULL);
    gtk_buildable_set_name(GTK_BUILDABLE(accent_button), "accent-button");

    gtk_grid_attach(GTK_GRID(grid), bg_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bg_button, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), fg_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), fg_button, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), accent_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), accent_button, 1, 2, 1, 1);

    GdkRGBA bg_rgba;
    GdkRGBA fg_rgba;
    GdkRGBA accent_rgba;
    gdk_rgba_parse(&bg_rgba, context->ui_config.colors.background);
    gdk_rgba_parse(&fg_rgba, context->ui_config.colors.foreground);
    gdk_rgba_parse(&accent_rgba, context->ui_config.colors.accent);

    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(bg_button), &bg_rgba);
    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(fg_button), &fg_rgba);
    gtk_color_dialog_button_set_rgba(GTK_COLOR_DIALOG_BUTTON(accent_button), &accent_rgba);

    g_signal_connect(bg_button, "notify::rgba", G_CALLBACK(update_color_from_button), context);
    g_signal_connect(fg_button, "notify::rgba", G_CALLBACK(update_color_from_button), context);
    g_signal_connect(accent_button, "notify::rgba", G_CALLBACK(update_color_from_button), context);

    GtkWidget *frame = gtk_frame_new("Theme colors");
    gtk_frame_set_child(GTK_FRAME(frame), grid);
    gtk_widget_set_hexpand(frame, TRUE);

    return frame;
}

static GtkWidget *build_search_area(DeerUiContext *context) {
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);

    GtkWidget *search_entry = gtk_entry_new_with_buffer(context->search_buffer);
    gtk_widget_set_hexpand(search_entry, TRUE);
    GtkWidget *search_button = gtk_button_new_with_label("Search & Query");
    gtk_widget_set_hexpand(search_button, FALSE);
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_activate), context);

    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(controls), search_entry);
    gtk_box_append(GTK_BOX(controls), search_button);
    gtk_box_append(GTK_BOX(search_box), controls);

    GtkWidget *frame = gtk_frame_new("Main search area");
    gtk_frame_set_child(GTK_FRAME(frame), search_box);

    return frame;
}

static GtkWidget *build_data_entry_area(DeerUiContext *context) {
    GtkWidget *entry_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    GtkWidget *data_entry = gtk_entry_new_with_buffer(context->data_entry_buffer);
    gtk_widget_set_hexpand(data_entry, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(data_entry), "Enter data to persist");

    GtkWidget *save_button = gtk_button_new_with_label("Save Item");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_data_submit), context);

    gtk_box_append(GTK_BOX(entry_box), data_entry);
    gtk_box_append(GTK_BOX(entry_box), save_button);

    GtkWidget *frame = gtk_frame_new("Saved item entry");
    gtk_frame_set_child(GTK_FRAME(frame), entry_box);

    return frame;
}

static GtkWidget *build_main_window(GtkApplication *app, DeerUiContext *context) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "DeerBourne");
    gtk_window_set_default_size(GTK_WINDOW(window), 960, 720);

    GtkWidget *header = gtk_header_bar_new();
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(header), gtk_label_new(context->ui_config.theme_name));
    gtk_window_set_titlebar(GTK_WINDOW(window), header);

    GtkWidget *root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_top(root, 12);
    gtk_widget_set_margin_bottom(root, 12);
    gtk_widget_set_margin_start(root, 12);
    gtk_widget_set_margin_end(root, 12);

    GtkWidget *content_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(content_grid), 12);
    gtk_grid_set_column_spacing(GTK_GRID(content_grid), 12);

    GtkWidget *search_area = build_search_area(context);
    GtkWidget *history_view = build_list_view(context->history, "History view");
    GtkWidget *saved_view = build_list_view(context->saved_items, "Saved items");
    GtkWidget *data_entry_area = build_data_entry_area(context);
    GtkWidget *color_controls = build_color_controls(context);

    gtk_grid_attach(GTK_GRID(content_grid), search_area, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(content_grid), history_view, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(content_grid), saved_view, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(content_grid), data_entry_area, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(content_grid), color_controls, 1, 2, 1, 1);

    gtk_box_append(GTK_BOX(root), content_grid);
    gtk_window_set_child(GTK_WINDOW(window), root);

    apply_theme(window, context);
    return window;
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    DeerUiContext *context = (DeerUiContext *)user_data;
    GtkWidget *window = build_main_window(app, context);
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
