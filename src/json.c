#include "cJSON.h"
#include "../nob.h"

cJSON *awq_json_get_nested(cJSON *root, const char *path) {
  cJSON *found = NULL;

  Nob_String_View sv = nob_sv_from_cstr(path);
  Nob_String_View left = nob_sv_chop_by_delim(&sv, '.');

  found = cJSON_GetObjectItem(root, nob_temp_sv_to_cstr(left));
  if (!found)
    return NULL;

  // Recursive call, if there is path left
  if (strcmp(nob_temp_sv_to_cstr(sv), "") != 0) {
    found = awq_json_get_nested(found, nob_temp_sv_to_cstr(sv));
  }

  return found;
}
