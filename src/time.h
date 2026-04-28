#ifndef AWQAT_UTILS_H_
#define AWQAT_UTILS_H_

#include "../nob.h"

typedef struct {
  int time_h;
  int time_m;
} Time;

Nob_String_Builder get_date_now();
Time get_time_now();
Time parse_time(const char *time);
Time time_substract(Time a, Time b);

#endif // AWQAT_UTILS_H_
