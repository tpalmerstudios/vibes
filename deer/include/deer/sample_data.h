/**
 * @file sample_data.h
 * @brief Helpers for loading showcase records into the Deer UI.
 */

#ifndef DEERBOURNE_DEER_SAMPLE_DATA_H
#define DEERBOURNE_DEER_SAMPLE_DATA_H

#include <glib.h>

/** Default relative path to bundled sample records. */
#define DEER_SAMPLE_DATA_PATH "data/sample_records.json"

/**
 * @brief Simple record used to render showcase data in the Deer UI.
 */
typedef struct {
    gchar *title;   /**< Human-friendly title for the entry. */
    gchar *summary; /**< Short description shown in list views. */
    gchar *details; /**< Multiline formatted details for the detail pane. */
    gchar *category; /**< Primary category for the entry (person, car, home). */
    GPtrArray *relationships; /**< Related entries rendered in the relationships pane. */
} DeerSampleRecord;

/**
 * @brief Load sample data from disk or a built-in fallback set.
 *
 * When a JSON file is available and readable, the contents are parsed and
 * transformed into @ref DeerSampleRecord structures. Each record owns its
 * strings and will be freed by @ref deer_sample_data_free.
 *
 * @param path Optional JSON file path; when NULL a project-relative default is used.
 * @param out_records Output pointer to an array of owned @ref DeerSampleRecord pointers.
 * @return true on success, false when allocation fails.
 */
bool deer_sample_data_load(const char *path, GPtrArray **out_records);

/**
 * @brief Release an array of @ref DeerSampleRecord entries.
 *
 * @param records Owned array returned by @ref deer_sample_data_load.
 */
void deer_sample_data_free(GPtrArray *records);

#endif // DEERBOURNE_DEER_SAMPLE_DATA_H
