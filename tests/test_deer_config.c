/**
 * @file test_deer_config.c
 * @brief Basic UI configuration test coverage for Deer defaults.
 */

#include <assert.h>
#include <stddef.h>

#include "deer/ui_config.h"

int main(void) {
    DeerUiConfig config = {0};
    deer_ui_config_use_defaults(&config);

    assert(config.theme_name != NULL);
    assert(config.colors.background != NULL);
    assert(config.colors.foreground != NULL);
    assert(config.colors.accent != NULL);

    deer_free_ui_config(&config);
    return 0;
}
