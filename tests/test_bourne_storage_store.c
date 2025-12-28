#include "bourne/storage.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    assert(bourne_storage_init());

    size_t before = bourne_storage_record_count();

    const char payload[] =
        "{\"title\":\"User note\",\"summary\":\"quick link between records\",\"tags\":[\"user\"]}";
    char status[128] = {0};
    assert(bourne_storage_store(payload, status, sizeof(status)));
    assert(status[0] != '\0');

    size_t after = bourne_storage_record_count();
    assert(after == before + 1U);

    char buffer[256] = {0};
    size_t matches = bourne_storage_search("User note", buffer, sizeof(buffer));
    assert(matches >= 1U);
    assert(strstr(buffer, "User note") != NULL);

    bourne_storage_shutdown();
    return 0;
}
