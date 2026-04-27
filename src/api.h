#ifndef AWQAT_API_H_
#define AWQAT_API_H_

#include <stddef.h>
#include "../nob.h"

typedef struct {
  char *data;
  size_t len;
} Response;

typedef struct {
  Nob_String_Builder name;
  Nob_String_Builder value;
} Param;

typedef struct {
  Param *items;
  size_t count;
  size_t capacity;
} Params;

int awq_fetch(const char *main_url, const char *path_params, const Params *query_params, Nob_String_Builder *response);

int awq_add_param(Params *params, const char *name, const char *value);

int awq_delete_params(Params *params);

int awq_get_user_coord(Params *params);

#endif // AWQAT_API_H_
