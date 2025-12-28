/**
 * @file ui_layout.c
 * @brief GTK layout builders for Deer.
 */

#include "deer/ui_layout.h"

#include "deer/client.h"

#include <gio/gio.h>

static void list_item_setup(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer user_data);
static void list_item_bind(GtkSignalListItemFactory *factory, GtkListItem *list_item, gpointer user_data);
static void on_record_selection_changed(GtkSelectionModel *model, guint position, guint n_items, gpointer user_data);
static void on_connection_test(GtkButton *button, gpointer user_data);
static GtkWidget *build_connection_area(DeerUiContext *context);
static void update_connection_status(DeerUiContext *context, gboolean success, const char *detail);
static void deer_ui_apply_layout(DeerUiContext *context);
static void deer_ui_bind_menu(GtkApplication *app, DeerUiContext *context);
static void deer_ui_toggle_layout_action(GSimpleAction *action, GVariant *state, gpointer user_data);
static void deer_ui_reset_layout_action(GSimpleAction *action, GVariant *parameter, gpointer user_data);
static void deer_ui_apply_layout(DeerUiContext *context);
static void deer_ui_bind_menu(GtkApplication *app, DeerUiContext *context);
static void update_relationships_view(DeerUiContext *context, DeerSampleRecord *record);

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

static void update_connection_status(DeerUiContext *context, gboolean success, const char *detail) {
    if (!context || !context->connection_status_label) {
        return;
    }

    const char *status = success ? "Reachable" : "Unavailable";
    gchar *label = g_strdup_printf("%s:%u • %s%s%s",
                                   context->client.config.host,
                                   context->client.config.port,
                                   status,
                                   (detail && detail[0] != '\0') ? " – " : "",
                                   detail ? detail : "");
    gtk_label_set_text(GTK_LABEL(context->connection_status_label), label);
    g_free(label);
}

static void update_relationships_view(DeerUiContext *context, DeerSampleRecord *record) {
    if (!context || !context->relationships) {
        return;
    }

    guint existing = gtk_string_list_get_n_items(context->relationships);
    gtk_string_list_splice(context->relationships, 0, existing, NULL);

    if (!record || !record->relationships || record->relationships->len == 0U) {
        gtk_string_list_append(context->relationships, "No relationship data yet");
        return;
    }

    for (guint i = 0; i < record->relationships->len; ++i) {
        const char *relation = g_ptr_array_index(record->relationships, i);
        gtk_string_list_append(context->relationships, relation ? relation : "Related entry");
    }
}

static void deer_ui_apply_layout(DeerUiContext *context) {
    if (!context) {
        return;
    }

    if (context->search_area) {
        gtk_widget_set_visible(context->search_area, context->ui_config.layout.show_search);
    }
    if (context->history_view) {
        gtk_widget_set_visible(context->history_view, context->ui_config.layout.show_history);
    }
    if (context->saved_view) {
        gtk_widget_set_visible(context->saved_view, context->ui_config.layout.show_saved_items);
    }
    if (context->data_entry_area) {
        gtk_widget_set_visible(context->data_entry_area, context->ui_config.layout.show_data_entry);
    }
    if (context->color_controls) {
        gtk_widget_set_visible(context->color_controls, context->ui_config.layout.show_theme_controls);
    }
    if (context->connection_area) {
        gtk_widget_set_visible(context->connection_area, context->ui_config.layout.show_connection);
    }
    if (context->sample_area) {
        gtk_widget_set_visible(context->sample_area, context->ui_config.layout.show_sample_browser);
    }
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

static void on_record_selection_changed(GtkSelectionModel *model, guint position, guint n_items, gpointer user_data) {
    (void)position;
    (void)n_items;
    DeerUiContext *context = (DeerUiContext *)user_data;
    if (!context || !context->sample_records || !context->sample_detail_buffer) {
        return;
    }

    guint selected = gtk_single_selection_get_selected(GTK_SINGLE_SELECTION(model));
    if (selected == GTK_INVALID_LIST_POSITION || selected >= context->sample_records->len) {
        gtk_text_buffer_set_text(context->sample_detail_buffer, "Select a record to view its descriptors.", -1);
        return;
    }

    DeerSampleRecord *record = g_ptr_array_index(context->sample_records, selected);
    if (!record) {
        gtk_text_buffer_set_text(context->sample_detail_buffer, "No details available.", -1);
        return;
    }

    gtk_text_buffer_set_text(context->sample_detail_buffer, record->details ? record->details : "No details available.", -1);
    update_relationships_view(context, record);
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

static void on_connection_test(GtkButton *button, gpointer user_data) {
    (void)button;
    DeerUiContext *context = (DeerUiContext *)user_data;
    if (!context) {
        return;
    }

    char message[128] = {0};
    gboolean success = deer_client_test_connection(&context->client, message, sizeof(message));
    const char *detail = (message[0] != '\0') ? message : (success ? "Handshake succeeded" : "Probe failed");
    update_connection_status(context, success, detail);
    on_history_append(context, detail);
}

static void on_search_activate(GtkButton *button, gpointer user_data) {
    (void)button;
    DeerUiContext *context = (DeerUiContext *)user_data;
    const char *query = gtk_entry_buffer_get_text(context->search_buffer);
    on_history_append(context, query);
    char response[1024] = {0};
    if (deer_query_data(&context->client, query, response, sizeof(response))) {
        on_saved_append(context, response);
    } else {
        on_saved_append(context, "Search failed");
    }
}

static void on_data_submit(GtkButton *button, gpointer user_data) {
    (void)button;
    DeerUiContext *context = (DeerUiContext *)user_data;
    const char *payload = gtk_entry_buffer_get_text(context->data_entry_buffer);
    char response[256] = {0};
    if (deer_submit_data(&context->client, payload, response, sizeof(response))) {
        on_saved_append(context, response[0] ? response : "Stored");
    } else {
        on_saved_append(context, "Submit failed");
    }
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

    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_editable_set_buffer(GTK_EDITABLE(search_entry), context->search_buffer);
    gtk_widget_set_hexpand(search_entry, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Search across people, cars, and homes");
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

static GtkWidget *build_connection_area(DeerUiContext *context) {
    GtkWidget *endpoint_label = gtk_label_new(NULL);
    gchar *endpoint = g_strdup_printf("Endpoint: %s:%u", context->client.config.host, context->client.config.port);
    gtk_label_set_text(GTK_LABEL(endpoint_label), endpoint);
    g_free(endpoint);

    GtkWidget *status_label = gtk_label_new("Connection not tested");
    context->connection_status_label = status_label;

    GtkWidget *test_button = gtk_button_new_with_label("Test connection");
    g_signal_connect(test_button, "clicked", G_CALLBACK(on_connection_test), context);

    GtkWidget *controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(controls), endpoint_label);
    gtk_box_append(GTK_BOX(controls), test_button);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
    gtk_box_append(GTK_BOX(box), controls);
    gtk_box_append(GTK_BOX(box), status_label);

    GtkWidget *frame = gtk_frame_new("Network connectivity");
    gtk_frame_set_child(GTK_FRAME(frame), box);
    gtk_widget_set_hexpand(frame, TRUE);

    update_connection_status(context, FALSE, "Not yet tested");
    return frame;
}

static GtkWidget *build_sample_data_area(DeerUiContext *context) {
    GtkWidget *factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup", G_CALLBACK(list_item_setup), NULL);
    g_signal_connect(factory, "bind", G_CALLBACK(list_item_bind), NULL);

    GtkListView *list_view = GTK_LIST_VIEW(gtk_list_view_new(
        GTK_SELECTION_MODEL(context->sample_selection), GTK_LIST_ITEM_FACTORY(factory)));
    g_signal_connect(context->sample_selection, "selection-changed", G_CALLBACK(on_record_selection_changed), context);
    GtkWidget *list_scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(list_scroller), GTK_WIDGET(list_view));
    gtk_widget_set_vexpand(list_scroller, TRUE);
    GtkWidget *list_frame = gtk_frame_new("Records");
    gtk_frame_set_child(GTK_FRAME(list_frame), list_scroller);

    GtkWidget *detail_view = gtk_text_view_new_with_buffer(context->sample_detail_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(detail_view), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(detail_view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(detail_view), 8);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(detail_view), 8);
    GtkWidget *detail_scroller = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(detail_scroller), detail_view);
    gtk_widget_set_hexpand(detail_scroller, TRUE);
    gtk_widget_set_vexpand(detail_scroller, TRUE);
    GtkWidget *detail_frame = gtk_frame_new("Descriptors");
    gtk_frame_set_child(GTK_FRAME(detail_frame), detail_scroller);

    GtkWidget *relationships_frame = build_list_view(context->relationships, "Relationships");
    context->relationship_view = relationships_frame;

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_grid_attach(GTK_GRID(grid), list_frame, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), relationships_frame, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), detail_frame, 2, 0, 1, 1);

    GtkWidget *frame = gtk_frame_new("Structured data explorer");
    gtk_frame_set_child(GTK_FRAME(frame), grid);
    gtk_widget_set_hexpand(frame, TRUE);
    gtk_widget_set_vexpand(frame, TRUE);

    return frame;
}

static void deer_ui_toggle_layout_action(GSimpleAction *action, GVariant *state, gpointer user_data) {
    DeerUiContext *context = (DeerUiContext *)user_data;
    if (!context || !state || !action) {
        return;
    }

    const char *name = g_action_get_name(G_ACTION(action));
    gboolean enabled = g_variant_get_boolean(state);

    if (g_strcmp0(name, "layout.search") == 0) {
        context->ui_config.layout.show_search = enabled;
    } else if (g_strcmp0(name, "layout.history") == 0) {
        context->ui_config.layout.show_history = enabled;
    } else if (g_strcmp0(name, "layout.saved") == 0) {
        context->ui_config.layout.show_saved_items = enabled;
    } else if (g_strcmp0(name, "layout.data_entry") == 0) {
        context->ui_config.layout.show_data_entry = enabled;
    } else if (g_strcmp0(name, "layout.theme_controls") == 0) {
        context->ui_config.layout.show_theme_controls = enabled;
    } else if (g_strcmp0(name, "layout.connection") == 0) {
        context->ui_config.layout.show_connection = enabled;
    } else if (g_strcmp0(name, "layout.sample_browser") == 0) {
        context->ui_config.layout.show_sample_browser = enabled;
    }

    g_simple_action_set_state(action, state);
    deer_ui_apply_layout(context);
}

static void deer_ui_reset_layout_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    (void)action;
    (void)parameter;
    DeerUiContext *context = (DeerUiContext *)user_data;
    if (!context) {
        return;
    }

    context->ui_config.layout = context->default_layout;

    const struct {
        const char *name;
        bool enabled;
    } actions[] = {
        {"layout.search", context->ui_config.layout.show_search},
        {"layout.history", context->ui_config.layout.show_history},
        {"layout.saved", context->ui_config.layout.show_saved_items},
        {"layout.data_entry", context->ui_config.layout.show_data_entry},
        {"layout.theme_controls", context->ui_config.layout.show_theme_controls},
        {"layout.connection", context->ui_config.layout.show_connection},
        {"layout.sample_browser", context->ui_config.layout.show_sample_browser},
    };

    if (context->app) {
        for (size_t i = 0; i < sizeof(actions) / sizeof(actions[0]); ++i) {
            GAction *found = g_action_map_lookup_action(G_ACTION_MAP(context->app), actions[i].name);
            if (found) {
                g_simple_action_set_state(G_SIMPLE_ACTION(found), g_variant_new_boolean(actions[i].enabled));
            }
        }
    }

    deer_ui_apply_layout(context);
}

static void deer_ui_bind_menu(GtkApplication *app, DeerUiContext *context) {
    if (!app || !context) {
        return;
    }

    const struct {
        const char *name;
        const char *label;
        bool *state;
    } toggle_actions[] = {
        {"layout.search", "Search", &context->ui_config.layout.show_search},
        {"layout.history", "History", &context->ui_config.layout.show_history},
        {"layout.saved", "Saved Items", &context->ui_config.layout.show_saved_items},
        {"layout.data_entry", "Data Entry", &context->ui_config.layout.show_data_entry},
        {"layout.theme_controls", "Theme", &context->ui_config.layout.show_theme_controls},
        {"layout.connection", "Connectivity", &context->ui_config.layout.show_connection},
        {"layout.sample_browser", "Sample Data", &context->ui_config.layout.show_sample_browser},
    };

    for (size_t i = 0; i < sizeof(toggle_actions) / sizeof(toggle_actions[0]); ++i) {
        GSimpleAction *toggle = g_simple_action_new_stateful(toggle_actions[i].name,
                                                            NULL,
                                                            g_variant_new_boolean(*toggle_actions[i].state));
        g_signal_connect(toggle, "change-state", G_CALLBACK(deer_ui_toggle_layout_action), context);
        g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(toggle));
    }

    GSimpleAction *reset_action = g_simple_action_new("layout.reset", NULL);
    g_signal_connect(reset_action, "activate", G_CALLBACK(deer_ui_reset_layout_action), context);
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(reset_action));

    GMenu *layout_menu = g_menu_new();
    for (size_t i = 0; i < sizeof(toggle_actions) / sizeof(toggle_actions[0]); ++i) {
        gchar *detailed = g_strdup_printf("app.%s", toggle_actions[i].name);
        GMenuItem *item = g_menu_item_new(toggle_actions[i].label, detailed);
        g_menu_item_set_attribute(item, "toggle-type", "s", "check");
        g_menu_append_item(layout_menu, item);
        g_free(detailed);
        g_object_unref(item);
    }

    GMenuItem *reset_item = g_menu_item_new("Reset Layout", "app.layout.reset");
    g_menu_append_item(layout_menu, reset_item);
    g_object_unref(reset_item);

    GMenu *menubar = g_menu_new();
    g_menu_append_submenu(menubar, "Layout", G_MENU_MODEL(layout_menu));
    gtk_application_set_menubar(app, G_MENU_MODEL(menubar));

    g_object_unref(layout_menu);
    g_object_unref(menubar);
}

GtkWidget *deer_ui_build_main_window(GtkApplication *app, DeerUiContext *context) {
    if (context) {
        context->app = app;
    }
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
    GtkWidget *sample_area = build_sample_data_area(context);
    GtkWidget *history_view = build_list_view(context->history, "History view");
    GtkWidget *saved_view = build_list_view(context->saved_items, "Saved items");
    GtkWidget *data_entry_area = build_data_entry_area(context);
    GtkWidget *color_controls = build_color_controls(context);
    GtkWidget *connection_area = build_connection_area(context);

    context->search_area = search_area;
    context->sample_area = sample_area;
    context->history_view = history_view;
    context->saved_view = saved_view;
    context->data_entry_area = data_entry_area;
    context->color_controls = color_controls;
    context->connection_area = connection_area;

    gtk_grid_attach(GTK_GRID(content_grid), search_area, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(content_grid), sample_area, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(content_grid), history_view, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(content_grid), saved_view, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(content_grid), data_entry_area, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(content_grid), color_controls, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(content_grid), connection_area, 0, 4, 2, 1);

    gtk_box_append(GTK_BOX(root), content_grid);
    gtk_window_set_child(GTK_WINDOW(window), root);

    deer_ui_bind_menu(app, context);
    deer_ui_apply_layout(context);
    apply_theme(window, context);
    return window;
}
