/**
 * @file test_bourne_query_parser.c
 * @brief Validate Bourne query parsing and validation logic.
 */

#include <assert.h>
#include <string.h>

#include "bourne/query.h"

int main(void) {
    BourneCommand command = {0};

    assert(bourne_query_parse("PING\n", &command));
    assert(command.type == BOURNE_COMMAND_PING);
    assert(bourne_query_validate(&command));

    assert(bourne_query_parse("SEARCH adaptive reuse", &command));
    assert(command.type == BOURNE_COMMAND_SEARCH);
    assert(strcmp(command.argument, "adaptive reuse") == 0);
    assert(bourne_query_validate(&command));

    assert(bourne_query_parse("STORE {\"data\":1}", &command));
    assert(command.type == BOURNE_COMMAND_STORE);
    assert(bourne_query_validate(&command));

    assert(bourne_query_parse("SHUTDOWN", &command));
    assert(command.type == BOURNE_COMMAND_SHUTDOWN);
    assert(bourne_query_validate(&command));

    assert(!bourne_query_parse("UNKNOWN", &command));
    return 0;
}
