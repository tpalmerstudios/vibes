/**
 * @file test_bourne_config.c
 * @brief Basic configuration test coverage for Bourne defaults.
 */

#include <assert.h>

#include "bourne/config.h"

int main(void) {
    BourneServerConfig config;
    bourne_config_use_defaults(&config);

    assert(config.bind_address != NULL);
    assert(config.port != 0);
    assert(config.backlog > 0);
    assert(config.max_workers > 0);

    assert(bourne_config_validate(&config));
    return 0;
}
