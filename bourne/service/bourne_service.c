/**
 * @file bourne_service.c
 * @brief Entry point for the DeerBourne back-end service executable.
 */

#include "bourne.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    BourneServiceState state;
    BourneServerConfig config;
    bourne_config_use_defaults(&config);

    if (!bourne_service_init(&state, &config)) {
        fprintf(stderr, "bourne: failed to initialize service\n");
        return EXIT_FAILURE;
    }

    if (!bourne_service_start(&state)) {
        fprintf(stderr, "bourne: failed to start service\n");
        bourne_service_stop(&state);
        return EXIT_FAILURE;
    }

    bourne_service_run(&state);
    bourne_service_stop(&state);

    return EXIT_SUCCESS;
}
