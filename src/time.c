#include "time.h"

#define NOB_IMPLEMENTATION
#include "../nob.h"

Nob_String_Builder awq_get_date_now() {
  Nob_String_Builder sb = {0};

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);

  nob_sb_appendf(&sb, "%02d-%02d-%04d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
  return sb;
}

Time awq_get_time_now() {
  Time now = {0};

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);

  now.time_h = tm->tm_hour;
  now.time_m = tm->tm_min;

  return now;
}

#undef NOB_IMPLEMENTATION

Time awq_parse_time(const char *time) {
  Time t;
  int h, m;
  sscanf(time, "%d:%d", &h, &m);
  
  t.time_h = h;
  t.time_m = m;

  return t;
}

Time awq_time_substract(Time a, Time b) {
  Time t;

  t.time_h = a.time_h - b.time_h;
  t.time_m = a.time_m - b.time_m;

  return t;
}
