#include "../nob.h"
#include "time.h"
#include "cJSON.h"

typedef struct {
  const char* name;
  Time time;
  Time diff_now;
} Prayer;

typedef struct {
  Nob_String_Builder method;
  Nob_String_Builder city;
  Time time_now;
  Prayer *next_prayer;
  Prayer prayers[5];
} Main;

int awq_process(Main *main_st, cJSON *aladhan_data);
int awq_cleanup(Main *main_st);
