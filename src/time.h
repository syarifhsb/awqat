#ifndef AWQAT_UTILS_H_
#define AWQAT_UTILS_H_

#include "../nob.h"

typedef struct {
  int time_h;
  int time_m;
} Time;

Nob_String_Builder awq_get_date_now();
Time awq_get_time_now();
Time awq_parse_time(const char *time);
Time awq_time_substract(Time a, Time b);

#endif // AWQAT_UTILS_H_
