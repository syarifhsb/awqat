#include "awq_process.h"
#include "cJSON.h"
#include "json.h"
#include "time.h"
#include "../nob.h"

int awq_process(Main *main_st, cJSON *aladhan_data) {
  cJSON *timings = awq_json_get_nested(aladhan_data, "data.timings");
  const char *method = awq_json_get_nested(aladhan_data, "data.meta.method.name")->valuestring;

  nob_sb_append_cstr(&main_st->method, method);
  nob_sb_append_null(&main_st->method);

  for (size_t i = 0; i < NOB_ARRAY_LEN(main_st->prayers); i++) {
    Time prayer_time = awq_parse_time(cJSON_GetObjectItem(timings, main_st->prayers[i].name)->valuestring);
    main_st->prayers[i].time = prayer_time;
    main_st->prayers[i].diff_now = awq_time_substract(prayer_time, main_st->time_now);
  }

  int min = INT_MAX;
  size_t min_idx = 0;

  for (size_t i = 0; i < NOB_ARRAY_LEN(main_st->prayers); i++) {
    int diff = main_st->prayers[i].diff_now.time_h * 60 +
      main_st->prayers[i].diff_now.time_m;
    if (diff > 0 && diff < min) {
      min = diff;
      min_idx = i;
    }
  }

  main_st->next_prayer = &main_st->prayers[min_idx];

  return 0;
}

int awq_cleanup(Main *main_st) {
  if (main_st->method.count > 0)
    nob_sb_free(main_st->method);

  if (main_st->city.count > 0)
    nob_sb_free(main_st->city);

  return 0;
}

