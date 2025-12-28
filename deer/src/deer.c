/**
 * @file deer.c
 * @brief Implementation for the DeerBourne front-end (Deer) communication layer.
 */

#include "deer.h"

#include <glib.h>

#define DEER_DEFAULT_THEME "DeerBourne Modern"
#define DEER_DEFAULT_BACKGROUND "#0B1E2D"
#define DEER_DEFAULT_FOREGROUND "#E5E9F0"
#define DEER_DEFAULT_ACCENT "#5FD1FF"

void deer_init(DeerClient *client, const DeerClientConfig *config) {
    if (!client) {
        return;
    }

    client->config.host = (config && config->host) ? config->host : DEER_DEFAULT_HOST;
    client->config.port = (config && config->port != 0) ? config->port : DEER_DEFAULT_PORT;
}

void deer_ui_config_use_defaults(DeerUiConfig *config) {
    if (!config) {
        return;
    }

    config->theme_name = g_strdup(DEER_DEFAULT_THEME);
    config->colors.background = g_strdup(DEER_DEFAULT_BACKGROUND);
    config->colors.foreground = g_strdup(DEER_DEFAULT_FOREGROUND);
    config->colors.accent = g_strdup(DEER_DEFAULT_ACCENT);
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

    GKeyFile *key_file = g_key_file_new();
    GError *error = NULL;
    gboolean loaded = g_key_file_load_from_file(key_file, path, G_KEY_FILE_NONE, &error);
    if (!loaded) {
        if (error) {
            g_warning("Failed to load UI config at %s: %s", path, error->message);
            g_error_free(error);
        }
        g_key_file_unref(key_file);
        return false;
    }

    g_free(out_config->theme_name);
    out_config->theme_name = g_key_file_get_string(key_file, "Theme", "Name", NULL);
    if (!out_config->theme_name) {
        out_config->theme_name = g_strdup(DEER_DEFAULT_THEME);
    }

    g_free(out_config->colors.background);
    g_free(out_config->colors.foreground);
    g_free(out_config->colors.accent);

    out_config->colors.background = g_key_file_get_string(key_file, "Colors", "Background", NULL);
    out_config->colors.foreground = g_key_file_get_string(key_file, "Colors", "Foreground", NULL);
    out_config->colors.accent = g_key_file_get_string(key_file, "Colors", "Accent", NULL);

    if (!out_config->colors.background) {
        out_config->colors.background = g_strdup(DEER_DEFAULT_BACKGROUND);
    }
    if (!out_config->colors.foreground) {
        out_config->colors.foreground = g_strdup(DEER_DEFAULT_FOREGROUND);
    }
    if (!out_config->colors.accent) {
        out_config->colors.accent = g_strdup(DEER_DEFAULT_ACCENT);
    }

    g_key_file_unref(key_file);
    return true;
}

void deer_free_ui_config(DeerUiConfig *config) {
    if (!config) {
        return;
    }

    g_free(config->theme_name);
    g_free(config->colors.background);
    g_free(config->colors.foreground);
    g_free(config->colors.accent);

    config->theme_name = NULL;
    config->colors.background = NULL;
    config->colors.foreground = NULL;
    config->colors.accent = NULL;
}

void deer_submit_data(const DeerClient *client, const char *payload) {
    (void)client;
    if (!payload) {
        return;
    }

    g_message("Submitting payload to Bourne: %s", payload);
}

const char *deer_query_data(const DeerClient *client, const char *query) {
    (void)client;
    static char response[256];
    if (!query) {
        return "No query provided";
    }

    g_snprintf(response, sizeof(response), "Query '%s' scheduled for processing", query);
    return response;
}
