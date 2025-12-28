/**
 * @file query.c
 * @brief Lightweight parser and validator for Bourne protocol commands.
 */

#define _POSIX_C_SOURCE 200809L

#include "bourne/query.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static void trim_newlines(char *text) {
    if (!text) {
        return;
    }
    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        --len;
    }
}

static const char *skip_spaces(const char *text) {
    while (text && *text != '\0' && isspace((unsigned char)*text)) {
        ++text;
    }
    return text;
}

static BourneCommandType parse_type(const char *token) {
    if (!token) {
        return BOURNE_COMMAND_INVALID;
    }

    if (strcasecmp(token, "PING") == 0) {
        return BOURNE_COMMAND_PING;
    }
    if (strcasecmp(token, "SEARCH") == 0) {
        return BOURNE_COMMAND_SEARCH;
    }
    if (strcasecmp(token, "STORE") == 0) {
        return BOURNE_COMMAND_STORE;
    }
    if (strcasecmp(token, "SHUTDOWN") == 0) {
        return BOURNE_COMMAND_SHUTDOWN;
    }

    return BOURNE_COMMAND_INVALID;
}

bool bourne_query_parse(const char *raw, BourneCommand *command) {
    if (!raw || !command) {
        return false;
    }

    memset(command, 0, sizeof(*command));

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "%s", raw);
    trim_newlines(buffer);

    const char *cursor = skip_spaces(buffer);
    if (*cursor == '\0') {
        return false;
    }

    char verb[32] = {0};
    size_t verb_len = 0U;
    while (*cursor != '\0' && !isspace((unsigned char)*cursor) && verb_len + 1U < sizeof(verb)) {
        verb[verb_len++] = *cursor++;
    }
    verb[verb_len] = '\0';

    command->type = parse_type(verb);
    cursor = skip_spaces(cursor);

    if (*cursor != '\0') {
        snprintf(command->argument, sizeof(command->argument), "%s", cursor);
    }

    return command->type != BOURNE_COMMAND_INVALID;
}

bool bourne_query_validate(const BourneCommand *command) {
    if (!command || command->type == BOURNE_COMMAND_INVALID) {
        return false;
    }

    if (command->type == BOURNE_COMMAND_SEARCH || command->type == BOURNE_COMMAND_STORE) {
        return command->argument[0] != '\0';
    }

    return true;
}
