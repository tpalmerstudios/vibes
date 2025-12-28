/**
 * @file client.c
 * @brief Lightweight client wiring for the Deer front-end.
 */

#include "deer/client.h"

#include <stdio.h>
#include <string.h>

#define DEER_DEFAULT_HOST "127.0.0.1"
#define DEER_DEFAULT_PORT 5555

void deer_init(DeerClient *client, const DeerClientConfig *config) {
    if (!client) {
        return;
    }

    client->config.host = (config && config->host) ? config->host : DEER_DEFAULT_HOST;
    client->config.port = (config && config->port != 0) ? config->port : DEER_DEFAULT_PORT;
}

void deer_submit_data(const DeerClient *client, const char *payload) {
    (void)client;
    if (!payload) {
        return;
    }

    printf("Submitting payload to Bourne: %s\n", payload);
}

const char *deer_query_data(const DeerClient *client, const char *query) {
    (void)client;
    static char response[256];
    if (!query) {
        return "No query provided";
    }

    snprintf(response, sizeof(response), "Query '%s' scheduled for processing", query);
    return response;
}
