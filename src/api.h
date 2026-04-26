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

int awq_fetch(const char *main_url, const char *path_params, const Params *query_params, Nob_String_Builder *response);

Params awq_get_user_coord();

#endif // AWQAT_API_H_
