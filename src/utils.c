#include <time.h>

#define NOB_IMPLEMENTATION
#include "../nob.h"

Nob_String_Builder get_date_now() {
  Nob_String_Builder sb = {0};

  time_t t = time(NULL);
  struct tm *tm = localtime(&t);

  nob_sb_appendf(&sb, "%02d-%02d-%04d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
  return sb;
}
