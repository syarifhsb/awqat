#ifndef AWQAT_JSON_H_
#define AWQAT_JSON_H_

#include "cJSON.h"

// traverse nested object using dot-delimited path (e.g. "data.timings.Fajr")
// returns NULL if any key in the path is not found
cJSON *awq_json_get_nested(cJSON *root, const char *path);

#endif // AWQAT_JSON_H_
