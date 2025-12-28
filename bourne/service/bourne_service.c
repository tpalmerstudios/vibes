/**
 * @file bourne_service.c
 * @brief Entry point for the DeerBourne back-end service executable.
 */

#include "bourne.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    BourneServer server;
    BourneServerConfig config = {
        .bind_address = "127.0.0.1",
        .port = 5555,
        .backlog = 8,
    };

    if (!bourne_init(&server, &config)) {
        fprintf(stderr, "bourne: failed to initialize service\n");
        return EXIT_FAILURE;
    }

    bourne_run(&server);
    bourne_shutdown(&server);

    return EXIT_SUCCESS;
}
