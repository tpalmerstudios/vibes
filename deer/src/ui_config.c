/**
 * @file ui_config.c
 * @brief UI configuration helpers for Deer.
 */

#include "deer/ui_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    /*
     * Parsing is intentionally deferred; the file is opened to validate access
     * and will be processed in a later iteration when the format stabilizes.
     */
    fclose(file);
    return false;
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
