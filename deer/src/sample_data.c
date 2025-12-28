/**
 * @file sample_data.c
 * @brief Sample data loader for the Deer UI showcase.
 */

#include "deer/sample_data.h"

#include <stdio.h>
#include <string.h>

#ifdef DEER_HAVE_JSON_GLIB
#include <json-glib/json-glib.h>
#endif

static GPtrArray *copy_relationships(const char *const *relationships) {
    GPtrArray *array = g_ptr_array_new_with_free_func(g_free);
    if (!array) {
        return NULL;
    }

    if (!relationships) {
        return array;
    }

    for (size_t i = 0; relationships[i] != NULL; ++i) {
        g_ptr_array_add(array, g_strdup(relationships[i]));
    }

    return array;
}

static DeerSampleRecord *deer_sample_record_new(const char *title, const char *summary, const char *details,
                                               const char *category, const char *const *relationships) {
    DeerSampleRecord *record = g_new0(DeerSampleRecord, 1);
    if (!record) {
        return NULL;
    }

    record->title = g_strdup(title ? title : "Untitled");
    record->summary = g_strdup(summary ? summary : "No summary");
    record->details = g_strdup(details ? details : "No details available.");
    record->category = g_strdup(category ? category : "uncategorized");
    record->relationships = copy_relationships(relationships);
    return record;
}

static void deer_sample_record_free(DeerSampleRecord *record) {
    if (!record) {
        return;
    }

    g_free(record->title);
    g_free(record->summary);
    g_free(record->details);
    g_free(record->category);
    if (record->relationships) {
        g_ptr_array_free(record->relationships, TRUE);
    }
    g_free(record);
}

static void append_builtin_records(GPtrArray *records) {
    static const struct {
        const char *title;
        const char *summary;
        const char *details;
        const char *category;
        const char *const *relationships;
    } BUILTIN_DATA[] = {
        {"Person · Arianna Patel", "Urban planner focused on adaptive reuse and transit equity.",
         "Type: Person\n"
         "Appearance: Black hair with copper highlights, 167cm, structured layers\n"
         "Possessions: Folding e-bike, drafting tablet\n"
         "Business interest: North Loop Studio — partner\n"
         "Interests: Transit equity, material reuse\n",
         "person",
         (const char *const[]){"Collaborates with: Person · Felix Moreno (analytics on corridor pilots)",
                               "Advises: Home · Alder Row (mobility plan)",
                               NULL}},
        {"Person · Felix Moreno", "CTO translating sensor data into neighborhood analytics.",
         "Type: Person\n"
         "Appearance: Silver undercut, 180cm, monochrome palette\n"
         "Possessions: Mechanical keyboard, gravel bike\n"
         "Business interest: SignalGrid Labs — CTO\n"
         "Interests: Edge compute, privacy-first ML\n",
         "person",
         (const char *const[]){"Maintains: Car · Vireo SX (telemetry dev mule)",
                               "Partners with: Person · Arianna Patel (data layer)",
                               NULL}},
        {"Person · Priya Deshmukh", "Hospitality designer crafting calming interiors.",
         "Type: Person\n"
         "Appearance: Warm brown curls, 160cm, linen-forward style\n"
         "Possessions: Material swatch library, analog camera\n"
         "Business interest: Lumen & Fern — principal designer\n"
         "Interests: Restorative spaces, sensory design\n",
         "person",
         (const char *const[]){"Curates: Home · Harbor Loft (interior mood)", "Commutes with: Car · Lumen Tourer", NULL}},
        {"Car · Solstice LX", "2021 plug-in hybrid sedan in satin graphite with panoramic roof.",
         "Type: Car\n"
         "Owner: Mara King\n"
         "Appearance: Satin graphite, 19-inch wheels, adaptive lighting\n"
         "Year: 2021\n"
         "Notes: Plug-in hybrid, roof rack ready\n",
         "car",
         (const char *const[]){"Used by: Person · Arianna Patel (site visits)", NULL}},
        {"Car · Vireo SX", "2022 electric fastback in midnight blue tuned for range.",
         "Type: Car\n"
         "Owner: Diego P.\n"
         "Appearance: Midnight blue, flush door handles, aero kit\n"
         "Year: 2022\n"
         "Notes: 400+ km range, panoramic roof\n",
         "car",
         (const char *const[]){"Instrumented by: Person · Felix Moreno", NULL}},
        {"Car · Lumen Tourer", "2019 wagon in alpine white with modular cargo system.",
         "Type: Car\n"
         "Owner: Yara Chen\n"
         "Appearance: Alpine white, matte black trim, utility rails\n"
         "Year: 2019\n"
         "Notes: AWD, modular cargo bays\n",
         "car",
         (const char *const[]){"Shared with: Person · Priya Deshmukh", NULL}},
        {"Home · Willamette Craftsman", "1958 craftsman with sage-green exterior and oak details.",
         "Type: Home\n"
         "Location: Portland, OR\n"
         "Appearance: Sage green siding, charcoal trim, oak beams\n"
         "Year: 1958\n"
         "Possessions: Native garden, restored porch swing\n",
         "home",
         (const char *const[]){"Visited by: Person · Lionel Brooks (archival project)", NULL}},
        {"Home · Harbor Loft", "Adaptive reuse loft with exposed brick and soft amber lighting.",
         "Type: Home\n"
         "Location: Baltimore, MD\n"
         "Appearance: Exposed brick, charcoal steel, amber uplighting\n"
         "Year: 1910 shell, 2021 interior\n"
         "Possessions: Roof deck planters, modular shelving\n",
         "home",
         (const char *const[]){"Interior by: Person · Priya Deshmukh", NULL}},
        {"Home · Juniper Courtyard", "Modern courtyard home wrapped in juniper and glass.",
         "Type: Home\n"
         "Location: Santa Fe, NM\n"
         "Appearance: Juniper cladding, glass courtyard walls\n"
         "Year: 2018\n"
         "Possessions: Pottery studio, climate battery greenhouse\n",
         "home",
         (const char *const[]){"Gardened by: Person · Arianna Patel (pilot plots)", NULL}},
        {"Person · Lionel Brooks", "Archivist capturing neighborhood histories for resilience plans.",
         "Type: Person\n"
         "Appearance: Salt-and-pepper fade, 175cm, indigo workwear\n"
         "Possessions: Field recorder, archival scanner\n"
         "Business interest: Civic Heritage Trust — researcher\n"
         "Interests: Oral histories, archival mapping\n",
         "person",
         (const char *const[]){"Documents: Home · Willamette Craftsman", NULL}},
        {"Car · Northstar Summit", "2020 SUV in storm gray tuned for trail support crews.",
         "Type: Car\n"
         "Owner: Callie Rivers\n"
         "Appearance: Storm gray, skid plates, amber auxiliary lights\n"
         "Year: 2020\n"
         "Notes: Off-road package, medical kit storage\n",
         "car",
         (const char *const[]){"Supports: Home · Alder Row (winter supply runs)", NULL}},
        {"Home · Alder Row", "Townhome trio with charcoal brick and copper balcony rails.",
         "Type: Home\n"
         "Location: Minneapolis, MN\n"
         "Appearance: Charcoal brick, copper rails, triple skylights\n"
         "Year: 2020\n"
         "Possessions: Shared green roof, integrated bike storage\n",
         "home",
         (const char *const[]){"Residents coordinate with: Person · Arianna Patel", "Fleet support from: Car · Northstar Summit", NULL}}
    };

    for (size_t i = 0; i < G_N_ELEMENTS(BUILTIN_DATA); ++i) {
        DeerSampleRecord *record = deer_sample_record_new(BUILTIN_DATA[i].title,
                                                         BUILTIN_DATA[i].summary,
                                                         BUILTIN_DATA[i].details,
                                                         BUILTIN_DATA[i].category,
                                                         BUILTIN_DATA[i].relationships);
        if (record) {
            g_ptr_array_add(records, record);
        }
    }
}

#ifdef DEER_HAVE_JSON_GLIB
static gchar *format_details_from_object(JsonObject *detail_object) {
    if (!detail_object) {
        return g_strdup("No details available.");
    }

    GString *builder = g_string_new(NULL);
    JsonObjectIter iter;
    const gchar *member_name;
    JsonNode *member_node;
    json_object_iter_init(&iter, detail_object);
    while (json_object_iter_next(&iter, &member_name, &member_node)) {
        if (JSON_NODE_HOLDS_VALUE(member_node)) {
            GValue value = G_VALUE_INIT;
            json_node_get_value(member_node, &value);
            gchar *value_str = g_strdup_value_contents(&value);
            g_string_append_printf(builder, "%s: %s\n", member_name, value_str);
            g_free(value_str);
            g_value_unset(&value);
        } else if (JSON_NODE_HOLDS_ARRAY(member_node)) {
            JsonArray *array = json_node_get_array(member_node);
            guint len = json_array_get_length(array);
            g_string_append_printf(builder, "%s: ", member_name);
            for (guint i = 0; i < len; ++i) {
                const gchar *item = json_array_get_string_element(array, i);
                g_string_append(builder, item ? item : "");
                if (i + 1U < len) {
                    g_string_append(builder, ", ");
                }
            }
            g_string_append(builder, "\n");
        }
    }

    return g_string_free(builder, FALSE);
}

static bool append_records_from_json(const char *path, GPtrArray *records) {
    g_autoptr(JsonParser) parser = json_parser_new();
    GError *error = NULL;
    if (!json_parser_load_from_file(parser, path, &error)) {
        if (error) {
            g_warning("Failed to parse sample data %s: %s", path, error->message);
            g_error_free(error);
        }
        return false;
    }

    JsonNode *root = json_parser_get_root(parser);
    if (!JSON_NODE_HOLDS_ARRAY(root)) {
        g_warning("Sample data %s is not a JSON array", path);
        return false;
    }

    JsonArray *array = json_node_get_array(root);
    guint length = json_array_get_length(array);
    for (guint i = 0; i < length; ++i) {
        JsonObject *object = json_array_get_object_element(array, i);
        if (!object) {
            continue;
        }

        const gchar *title = json_object_has_member(object, "title")
                                 ? json_object_get_string_member(object, "title")
                                 : "Untitled";
        const gchar *summary = json_object_has_member(object, "summary")
                                   ? json_object_get_string_member(object, "summary")
                                   : "No summary available";
        const gchar *category = json_object_has_member(object, "category")
                                    ? json_object_get_string_member(object, "category")
                                    : (json_object_has_member(object, "details")
                                           ? json_object_get_string_member(json_object_get_object_member(object, "details"),
                                                                           "type")
                                           : "uncategorized");

        gchar *details = NULL;
        if (json_object_has_member(object, "details")) {
            JsonObject *detail_object = json_object_get_object_member(object, "details");
            details = format_details_from_object(detail_object);
        }

        GPtrArray *relationships = g_ptr_array_new_with_free_func(g_free);
        if (json_object_has_member(object, "relationships")) {
            JsonArray *relationship_array = json_object_get_array_member(object, "relationships");
            guint rel_len = json_array_get_length(relationship_array);
            for (guint r = 0; r < rel_len; ++r) {
                const gchar *relation_text = json_array_get_string_element(relationship_array, r);
                g_ptr_array_add(relationships, g_strdup(relation_text ? relation_text : ""));
            }
        }

        if (!details) {
            details = g_strdup("No details available.");
        }

        DeerSampleRecord *record = deer_sample_record_new(title, summary, details, category, NULL);
        g_free(details);
        if (record) {
            if (relationships) {
                record->relationships = relationships;
            }
            g_ptr_array_add(records, record);
        } else if (relationships) {
            g_ptr_array_free(relationships, TRUE);
        }
    }

    return records->len > 0U;
}
#endif

bool deer_sample_data_load(const char *path, GPtrArray **out_records) {
    if (!out_records) {
        return false;
    }

    GPtrArray *records = g_ptr_array_new_with_free_func((GDestroyNotify)deer_sample_record_free);
    if (!records) {
        return false;
    }

    const char *candidate_path = path ? path : DEER_SAMPLE_DATA_PATH;

#ifdef DEER_HAVE_JSON_GLIB
    if (candidate_path && g_file_test(candidate_path, G_FILE_TEST_EXISTS)) {
        if (append_records_from_json(candidate_path, records)) {
            *out_records = records;
            return true;
        }
        g_ptr_array_set_size(records, 0);
    }
#endif

    append_builtin_records(records);
    *out_records = records;
    return true;
}

void deer_sample_data_free(GPtrArray *records) {
    if (!records) {
        return;
    }

    g_ptr_array_free(records, TRUE);
}
