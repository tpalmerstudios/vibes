/**
 * @file storage.c
 * @brief Persistent storage surface for JSON payloads.
 */

#define _POSIX_C_SOURCE 200809L

#include "bourne/storage.h"

#include "bourne/logging.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BourneRecord g_records[BOURNE_MAX_RECORDS];
static size_t g_record_count = 0U;

static const char *DEFAULT_SAMPLE_PATH = "data/sample_records.json";

static void bourne_set_message(char *message, size_t message_len, const char *format, size_t value) {
    if (!message || message_len == 0U) {
        return;
    }

    if (!format) {
        message[0] = '\0';
        return;
    }

    if (strstr(format, "%zu")) {
        snprintf(message, message_len, format, value);
    } else {
        snprintf(message, message_len, "%s", format);
    }
}

static void bourne_storage_reset(void) {
    memset(g_records, 0, sizeof(g_records));
    g_record_count = 0U;
}

static char *trim_quotes(char *text) {
    if (!text) {
        return NULL;
    }
    while (*text && isspace((unsigned char)*text)) {
        ++text;
    }
    if (*text == '"') {
        ++text;
    }

    char *end = text + strlen(text);
    while (end > text && (*(end - 1) == '"' || isspace((unsigned char)*(end - 1)))) {
        --end;
    }
    *end = '\0';
    return text;
}

static bool extract_field(const char *entry, const char *key, char *buffer, size_t buffer_len) {
    if (!entry || !key || !buffer || buffer_len == 0) {
        return false;
    }

    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *found = strstr(entry, pattern);
    if (!found) {
        return false;
    }

    const char *colon = strchr(found, ':');
    if (!colon) {
        return false;
    }

    const char *start = strchr(colon, '"');
    if (!start) {
        return false;
    }
    ++start;

    const char *end = strchr(start, '"');
    if (!end || end <= start) {
        return false;
    }

    size_t length = (size_t)(end - start);
    if (length >= buffer_len) {
        length = buffer_len - 1U;
    }

    memcpy(buffer, start, length);
    buffer[length] = '\0';
    return true;
}

static void trim_newlines_local(char *text) {
    if (!text) {
        return;
    }

    size_t len = strlen(text);
    while (len > 0 && (text[len - 1] == '\n' || text[len - 1] == '\r')) {
        text[len - 1] = '\0';
        --len;
    }
}

static void sanitize_payload(const char *payload, char *buffer, size_t buffer_len) {
    if (!payload || !buffer || buffer_len == 0) {
        return;
    }

    snprintf(buffer, buffer_len, "%s", payload);
    trim_newlines_local(buffer);
}

static bool parse_entry(const char *start, const char *end) {
    if (!start || !end || start >= end || g_record_count >= BOURNE_MAX_RECORDS) {
        return false;
    }

    size_t length = (size_t)(end - start);
    char *entry_text = (char *)malloc(length + 1U);
    if (!entry_text) {
        return false;
    }
    memcpy(entry_text, start, length);
    entry_text[length] = '\0';

    BourneRecord record = {0};
    (void)extract_field(entry_text, "title", record.title, sizeof(record.title));
    (void)extract_field(entry_text, "summary", record.summary, sizeof(record.summary));

    strncpy(record.raw, trim_quotes(entry_text), sizeof(record.raw) - 1U);
    g_records[g_record_count++] = record;

    free(entry_text);
    return true;
}

static void load_builtin_records(void) {
    static const struct {
        const char *title;
        const char *summary;
        const char *raw;
    } FALLBACK[] = {
        {"Person · Arianna Patel", "Urban planner focused on adaptive reuse and transit equity.",
         "{\"type\":\"person\",\"notes\":\"Prefers cool palettes\"}"},
        {"Person · Felix Moreno", "CTO translating sensor data into neighborhood analytics.",
         "{\"type\":\"person\",\"notes\":\"Midnight blue interfaces\"}"},
        {"Car · Solstice LX", "2021 plug-in hybrid sedan in satin graphite with panoramic roof.",
         "{\"type\":\"car\",\"year\":2021}"},
        {"Home · Harbor Loft", "Adaptive reuse loft with exposed brick and soft amber lighting.",
         "{\"type\":\"home\",\"location\":\"Baltimore\"}"},
    };

    for (size_t i = 0; i < sizeof(FALLBACK) / sizeof(FALLBACK[0]) && g_record_count < BOURNE_MAX_RECORDS; ++i) {
        BourneRecord record = {0};
        strncpy(record.title, FALLBACK[i].title, sizeof(record.title) - 1U);
        strncpy(record.summary, FALLBACK[i].summary, sizeof(record.summary) - 1U);
        strncpy(record.raw, FALLBACK[i].raw, sizeof(record.raw) - 1U);
        g_records[g_record_count++] = record;
    }
}

static bool load_from_buffer(const char *buffer) {
    if (!buffer) {
        return false;
    }

    int depth = 0;
    const char *entry_start = NULL;
    bool loaded = false;

    for (const char *cursor = buffer; *cursor != '\0'; ++cursor) {
        if (*cursor == '{') {
            if (depth == 0) {
                entry_start = cursor;
            }
            depth++;
        } else if (*cursor == '}') {
            depth--;
            if (depth == 0 && entry_start) {
                parse_entry(entry_start, cursor + 1);
                loaded = true;
                entry_start = NULL;
            }
        }
    }

    return loaded;
}

bool bourne_storage_load(const char *path) {
    if (!path) {
        return false;
    }

    FILE *file = fopen(path, "r");
    if (!file) {
        BOURNE_LOG_VERBOSE("storage: unable to open %s; using fallback data", path);
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }
    long size = ftell(file);
    rewind(file);

    if (size <= 0 || size > 100000L) {
        fclose(file);
        return false;
    }

    char *buffer = (char *)malloc((size_t)size + 1U);
    if (!buffer) {
        fclose(file);
        return false;
    }

    size_t read_len = fread(buffer, 1, (size_t)size, file);
    buffer[read_len] = '\0';
    fclose(file);

    bool loaded = load_from_buffer(buffer);
    free(buffer);
    return loaded;
}

bool bourne_storage_init(void) {
    bourne_storage_reset();

    if (!bourne_storage_load(DEFAULT_SAMPLE_PATH)) {
        load_builtin_records();
    }

    return g_record_count > 0U;
}

void bourne_storage_shutdown(void) {
    bourne_storage_reset();
}

size_t bourne_storage_record_count(void) {
    return g_record_count;
}

bool bourne_storage_store(const char *payload, char *message, size_t message_len) {
    if (!payload || payload[0] == '\0') {
        bourne_set_message(message, message_len, "No payload provided", 0U);
        return false;
    }

    if (g_record_count >= BOURNE_MAX_RECORDS) {
        bourne_set_message(message, message_len, "Storage full (%zu records)", g_record_count);
        return false;
    }

    BourneRecord record = {0};
    char local[sizeof(record.raw)];
    sanitize_payload(payload, local, sizeof(local));

    if (!extract_field(local, "title", record.title, sizeof(record.title))) {
        snprintf(record.title, sizeof(record.title), "User entry %zu", g_record_count + 1U);
    }

    if (!extract_field(local, "summary", record.summary, sizeof(record.summary))) {
        snprintf(record.summary, sizeof(record.summary), "%.120s", trim_quotes(local));
    }

    strncpy(record.raw, trim_quotes(local), sizeof(record.raw) - 1U);
    g_records[g_record_count++] = record;

    bourne_set_message(message, message_len, "Stored entry #%zu", g_record_count);
    return true;
}

static bool contains_case_insensitive(const char *haystack, const char *needle) {
    if (!haystack || !needle || needle[0] == '\0') {
        return false;
    }

    for (const char *h = haystack; *h; ++h) {
        const char *h_iter = h;
        const char *n_iter = needle;
        while (*h_iter && *n_iter && tolower((unsigned char)*h_iter) == tolower((unsigned char)*n_iter)) {
            ++h_iter;
            ++n_iter;
        }
        if (*n_iter == '\0') {
            return true;
        }
    }
    return false;
}

size_t bourne_storage_search(const char *query, char *buffer, size_t buffer_len) {
    if (!buffer || buffer_len == 0) {
        return 0U;
    }

    buffer[0] = '\0';

    if (!query || query[0] == '\0') {
        snprintf(buffer, buffer_len, "No query provided\n");
        return 0U;
    }

    size_t matches = 0U;
    for (size_t i = 0; i < g_record_count; ++i) {
        const BourneRecord *record = &g_records[i];
        if (contains_case_insensitive(record->title, query) || contains_case_insensitive(record->summary, query) ||
            contains_case_insensitive(record->raw, query)) {
            ++matches;
            char line[512];
            snprintf(line, sizeof(line), "%s — %s\n", record->title, record->summary);
            strncat(buffer, line, buffer_len - strlen(buffer) - 1U);
        }
    }

    if (matches == 0U) {
        snprintf(buffer, buffer_len, "No matches found for '%s'\n", query);
    }

    return matches;
}
