/**
 * @file deer.c
 * @brief Implementation for the DeerBourne front-end (Deer) communication layer.
 */

#include "deer.h"

#define DEER_DEFAULT_HOST "127.0.0.1"
#define DEER_DEFAULT_PORT 5555

void deer_init(DeerClient *client, const DeerClientConfig *config) {
    if (!client) {
        return;
    }

    client->config.host = (config && config->host) ? config->host : DEER_DEFAULT_HOST;
    client->config.port = (config && config->port != 0) ? config->port : DEER_DEFAULT_PORT;
}
