/**
 * @file ui_config.c
 * @brief UI configuration helpers for Deer.
 */

#include "deer/ui_config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define DEER_DEFAULT_LAYOUT_SHOW true

#define DEER_DEFAULT_THEME "DeerBourne Modern"
#define DEER_DEFAULT_BACKGROUND "#0B1E2D"
#define DEER_DEFAULT_FOREGROUND "#E5E9F0"
#define DEER_DEFAULT_ACCENT "#5FD1FF"

static char *deer_strdup(const char *source) {
    if (!source) {
        return NULL;
    }

    size_t length = strlen(source) + 1U;
    char *copy = (char *)malloc(length);
    if (!copy) {
        return NULL;
    }

    memcpy(copy, source, length);
    return copy;
}

void deer_ui_config_use_defaults(DeerUiConfig *config) {
    if (!config) {
        return;
    }

    config->theme_name = deer_strdup(DEER_DEFAULT_THEME);
    config->colors.background = deer_strdup(DEER_DEFAULT_BACKGROUND);
    config->colors.foreground = deer_strdup(DEER_DEFAULT_FOREGROUND);
    config->colors.accent = deer_strdup(DEER_DEFAULT_ACCENT);

    config->layout.show_search = DEER_DEFAULT_LAYOUT_SHOW;
    config->layout.show_history = DEER_DEFAULT_LAYOUT_SHOW;
    config->layout.show_saved_items = DEER_DEFAULT_LAYOUT_SHOW;
    config->layout.show_data_entry = DEER_DEFAULT_LAYOUT_SHOW;
    config->layout.show_theme_controls = DEER_DEFAULT_LAYOUT_SHOW;
    config->layout.show_connection = DEER_DEFAULT_LAYOUT_SHOW;
    config->layout.show_sample_browser = DEER_DEFAULT_LAYOUT_SHOW;
}

static char *deer_trim(char *text) {
    if (!text) {
        return text;
    }

    while (*text && isspace((unsigned char)*text)) {
        ++text;
    }

    char *end = text + strlen(text);
    while (end > text && isspace((unsigned char)*(end - 1))) {
        --end;
    }
    *end = '\0';
    return text;
}

static bool deer_parse_bool(const char *value, bool fallback) {
    if (!value) {
        return fallback;
    }

    if (strcasecmp(value, "true") == 0 || strcmp(value, "1") == 0 || strcasecmp(value, "yes") == 0) {
        return true;
    }

    if (strcasecmp(value, "false") == 0 || strcmp(value, "0") == 0 || strcasecmp(value, "no") == 0) {
        return false;
    }

    return fallback;
}

static void deer_apply_config_entry(const char *key, const char *value, DeerUiConfig *config) {
    if (!key || !value || !config) {
        return;
    }

    if (strcasecmp(key, "theme") == 0) {
        free(config->theme_name);
        config->theme_name = deer_strdup(value);
    } else if (strcasecmp(key, "color_background") == 0) {
        free(config->colors.background);
        config->colors.background = deer_strdup(value);
    } else if (strcasecmp(key, "color_foreground") == 0) {
        free(config->colors.foreground);
        config->colors.foreground = deer_strdup(value);
    } else if (strcasecmp(key, "color_accent") == 0) {
        free(config->colors.accent);
        config->colors.accent = deer_strdup(value);
    } else if (strcasecmp(key, "show_search") == 0) {
        config->layout.show_search = deer_parse_bool(value, config->layout.show_search);
    } else if (strcasecmp(key, "show_history") == 0) {
        config->layout.show_history = deer_parse_bool(value, config->layout.show_history);
    } else if (strcasecmp(key, "show_saved_items") == 0) {
        config->layout.show_saved_items = deer_parse_bool(value, config->layout.show_saved_items);
    } else if (strcasecmp(key, "show_data_entry") == 0) {
        config->layout.show_data_entry = deer_parse_bool(value, config->layout.show_data_entry);
    } else if (strcasecmp(key, "show_theme_controls") == 0) {
        config->layout.show_theme_controls = deer_parse_bool(value, config->layout.show_theme_controls);
    } else if (strcasecmp(key, "show_connection") == 0) {
        config->layout.show_connection = deer_parse_bool(value, config->layout.show_connection);
    } else if (strcasecmp(key, "show_sample_browser") == 0) {
        config->layout.show_sample_browser = deer_parse_bool(value, config->layout.show_sample_browser);
    }
}

bool deer_load_ui_config(const char *path, DeerUiConfig *out_config) {
    if (!out_config) {
        return false;
    }

    deer_free_ui_config(out_config);
    deer_ui_config_use_defaults(out_config);

    if (!path) {
        return false;
    }

    FILE *file = fopen(path, "r");
    if (!file) {
        fprintf(stderr, "Failed to load UI config at %s\n", path);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *equals = strchr(line, '=');
        if (!equals) {
            continue;
        }

        *equals = '\0';
        char *key = deer_trim(line);
        char *value = deer_trim(equals + 1);
        deer_apply_config_entry(key, value, out_config);
    }

    fclose(file);
    return true;
}

void deer_free_ui_config(DeerUiConfig *config) {
    if (!config) {
        return;
    }

    free(config->theme_name);
    free(config->colors.background);
    free(config->colors.foreground);
    free(config->colors.accent);

    config->theme_name = NULL;
    config->colors.background = NULL;
    config->colors.foreground = NULL;
    config->colors.accent = NULL;
}
