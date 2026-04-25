#ifndef AWQAT_API_H_
#define AWQAT_API_H_

#include <stddef.h>
#include "../nob.h"

typedef struct {
  char *data;
  size_t len;
} Response;

typedef struct {
  const char *name;
  const char *value;
} Param;

typedef struct {
  Param *items;
  size_t count;
  size_t capacity;
} Params;

int api_curl_get(const char *main_url, void *cb, const char *path_params, Params *query_params, Nob_String_Builder *response);

#endif // AWQAT_API_H_
