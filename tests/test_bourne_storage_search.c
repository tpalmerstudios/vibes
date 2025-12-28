/**
 * @file test_bourne_storage_search.c
 * @brief Basic coverage for Bourne storage search helpers.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "bourne/storage.h"

int main(void) {
    assert(bourne_storage_init());
    assert(bourne_storage_record_count() > 0U);

    char buffer[256] = {0};
    size_t matches = bourne_storage_search("Urban", buffer, sizeof(buffer));
    assert(matches > 0U);
    assert(strstr(buffer, "Urban") != NULL);

    memset(buffer, 0, sizeof(buffer));
    matches = bourne_storage_search("nonexistent-token", buffer, sizeof(buffer));
    assert(matches == 0U);
    assert(strstr(buffer, "No matches") != NULL);

    bourne_storage_shutdown();
    return 0;
}
