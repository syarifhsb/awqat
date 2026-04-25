#ifndef AWQAT_API_H_
#define AWQAT_API_H_

#include <stddef.h>

typedef struct {
  char *data;
  size_t len;
} Response;

typedef struct {
  const char *name;
  const char *value;
} Param;

typedef struct {
  Param **params;
  size_t count;
  size_t capacity;
} Params;

#endif // AWQAT_API_H_
