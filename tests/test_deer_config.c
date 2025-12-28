/**
 * @file test_deer_config.c
 * @brief Basic UI configuration test coverage for Deer defaults.
 */

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "deer/ui_config.h"

int main(void) {
    DeerUiConfig config = {0};
    deer_ui_config_use_defaults(&config);

    assert(config.theme_name != NULL);
    assert(config.colors.background != NULL);
    assert(config.colors.foreground != NULL);
    assert(config.colors.accent != NULL);
    assert(config.layout.show_search);
    assert(config.layout.show_history);
    assert(config.layout.show_saved_items);
    assert(config.layout.show_data_entry);
    assert(config.layout.show_theme_controls);
    assert(config.layout.show_connection);
    assert(config.layout.show_sample_browser);

    const char *path = "/tmp/deer_ui_config_test.cfg";
    FILE *file = fopen(path, "w+");
    assert(file != NULL);
    fprintf(file, "theme=Alt\nshow_history=false\nshow_saved_items=false\nshow_connection=false\ncolor_background=#111111\n");
    fclose(file);

    assert(deer_load_ui_config(path, &config));
    assert(strcmp(config.theme_name, "Alt") == 0);
    assert(strcmp(config.colors.background, "#111111") == 0);
    assert(!config.layout.show_history);
    assert(!config.layout.show_saved_items);
    assert(!config.layout.show_connection);

    unlink(path);

    deer_free_ui_config(&config);
    return 0;
}
