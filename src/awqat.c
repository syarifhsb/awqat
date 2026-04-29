#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "api.h"
#include "json.h"
#include "time.h"
#include "cJSON.h"

// Procedures calls
#include "awq_api_calls.h"

#include "../config.h"

#define NOB_UNSTRIP_PREFIX
#include "../nob.h"

int awq_show_usage(const char* app_name) {
  printf("Usage: %s [OPTION]\n", app_name);
  printf("Simple Prayer Times CLI\n");
  printf("Use without any option to give prayer times based on your location.\n\n");
  printf("Available options:\n");
  printf("  %-26s%s\n", "-m, --method=METHOD", "Select from the list of methods.");
  printf("  %-26s%s\n", "", "Methods can be integer or an alias.");
  printf("  %-26s%s\n", "-c, --city=CITY", "Get prayer times by city.");
  printf("  %-26s%s\n", "-n, --next", "Only display the next prayer.");
  printf("\n");
  printf("Methods:\n");
  for (size_t i = 0; i < NOB_ARRAY_LEN(methods); i++) {
    char tmp_buf[32];
    snprintf(tmp_buf, sizeof(tmp_buf), "%d, %s", methods[i].index, methods[i].alias);
    printf("  %-26s%s\n", tmp_buf, methods[i].full_name);
  }

  return 0;
}

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

typedef struct {
  int only_next;
} Options;

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

int awq_output(Main *main_st, Options *options) {
  if (options->only_next) {
    printf("%s: %02d:%02d\n",
        main_st->next_prayer->name,
        main_st->next_prayer->time.time_h,
        main_st->next_prayer->time.time_m);
    return 0;
  }

  if (main_st->method.count > 0) {
    printf("Prayer times calculation method: %s\n", main_st->method.items);
  }

  if (main_st->city.count > 0) {
    printf("City: %s\n", main_st->city.items);
  }

  printf("Prayer Times:\n");
  for (size_t i = 0; i < NOB_ARRAY_LEN(main_st->prayers); i++) {
    if (&main_st->prayers[i] == main_st->next_prayer)
      printf(BOLD RED "%-8s: %02d:%02d%s" RESET "\n",
          main_st->prayers[i].name,
          main_st->prayers[i].time.time_h,
          main_st->prayers[i].time.time_m,
          " *");
    else
      printf("%-8s: %02d:%02d\n",
          main_st->prayers[i].name,
          main_st->prayers[i].time.time_h,
          main_st->prayers[i].time.time_m);
  }

  return 0;
}

int awq_cleanup(Main *main_st) {
  if (main_st->method.count > 0)
    nob_sb_free(main_st->method);

  if (main_st->city.count > 0)
    nob_sb_free(main_st->city);

  return 0;
}

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

int main(int argc, char *argv[]) {
  Params awq_params = {0};
  char *city = NULL;

  int c;
  Options options = {0};

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"city",    required_argument, 0, 'c'},
      {"method",  required_argument, 0, 'm'},
      {"next",    no_argument,       0, 'n'},
      {"help",    no_argument,       0, 'h'},
      {0,         0,                 0,  0 }
    };

    c = getopt_long(argc, argv, "hm:c:n",
        long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
      // Case when long options do not have short equivalent.
      // Kept for future uses.
      case 0:
        printf("option %s", long_options[option_index].name);
        if (optarg)
          printf(" with arg %s", optarg);
        printf("\n");
        break;

      case 'c':
        printf("Search by city: %s\n", optarg);
        city = strdup(optarg);
        break;

      case 'm': {
          Method user_method = {0};
          int found = 0;

          char *end;
          long method_l = strtol(optarg, &end, 10);

          for (size_t i = 0; i < NOB_ARRAY_LEN(methods); i++)
          {
            if (*end == '\0') {
              // valid number — match by index
              if (methods[i].index == method_l) {
                found = 1;
                user_method = methods[i];
                break;
              }
            } else {
              // valid number — match by index
              if (strcmp(methods[i].alias, optarg) == 0) {
                found = 1;
                user_method = methods[i];
                break;
              }
            }
          }

          if (!found) {
            fprintf(stderr, "Method %s is not valid.\n", optarg);
            exit(EXIT_FAILURE);
          }

          char tmp_buf[8];
          snprintf(tmp_buf, sizeof(tmp_buf), "%d", user_method.index);
          awq_add_param(&awq_params, "method", tmp_buf);

          break;
        }

      case 'n':
        options.only_next = 1;
        break;

      case 'h':
        awq_show_usage(argv[0]);
        exit(EXIT_SUCCESS);
        break;

      case '?':
      default:
        fprintf(stderr, "Usage: %s [OPTION]\n", argv[0]);
        fprintf(stderr, "Seek help with 'awqat --help'.\n");
        exit(EXIT_FAILURE);
    }
  }

  Prayer prayers[5] = {{"Fajr",    {0,0}, {0,0}},
                       {"Dhuhr",   {0,0}, {0,0}},
                       {"Asr",     {0,0}, {0,0}},
                       {"Maghrib", {0,0}, {0,0}},
                       {"Isha",    {0,0}, {0,0}}};

  Main main_st = {0};
  memcpy(main_st.prayers, prayers, sizeof(prayers));
  main_st.time_now = awq_get_time_now();

  if (city) {
    main_st.city = awq_get_coord_by_city(&awq_params, city, NOMINATIM_URL, 1);
    free(city);
  } else {
    awq_get_user_coord(&awq_params, IP_API_URL);
  }

  cJSON *aladhan_data = awq_get_prayer_times(&awq_params, ALADHAN_TIMINGS_URL);
  awq_delete_params(&awq_params);

  awq_process(&main_st, aladhan_data);
  cJSON_Delete(aladhan_data);

  awq_output(&main_st, &options);
  awq_cleanup(&main_st);

  return 0;
}
