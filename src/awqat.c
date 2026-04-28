#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "api.h"
#include "utils.h"
#include "cJSON.h"
#include "json.h"

#include "../config.h"
#include "../nob.h"

int awq_show_usage(const char* app_name) {
  printf("Usage: %s [OPTION]\n", app_name);
  printf("Simple Prayer Times CLI\n");
  printf("Use without any option to give prayer times based on your location.\n\n");
  printf("Available options:\n");
  printf("  %-26s%s\n", "-m, --method=METHOD", "Select from the list of methods.");
  printf("  %-26s%s\n", "", "Methods can be integer or an alias.");
  printf("\n");
  printf("Methods:\n");
  for (size_t i = 0; i < NOB_ARRAY_LEN(methods); i++) {
    char tmp_buf[32];
    snprintf(tmp_buf, sizeof(tmp_buf), "%d, %s", methods[i].index, methods[i].alias);
    printf("  %-26s%s\n", tmp_buf, methods[i].full_name);
  }

  return 0;
}

int main(int argc, char *argv[]) {
  Params awq_params = {0};
  const char *city = NULL;

  int c;

  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"city",    required_argument, 0, 'c'},
      {"method",  required_argument, 0, 'm'},
      {"help",    no_argument,       0, 'h'},
      {0,         0,                 0,  0 }
    };

    c = getopt_long(argc, argv, "hm:c:",
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

          printf("Method chosen: %d-%s.\n", user_method.index, user_method.full_name);
          char tmp_buf[8];
          snprintf(tmp_buf, sizeof(tmp_buf), "%d", user_method.index);
          awq_add_param(&awq_params, "method", tmp_buf);

          break;
        }

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

  // if (optind < argc) {
  //   printf("non-option ARGV-elements: ");
  //   while (optind < argc)
  //     printf("%s ", argv[optind++]);
  //   printf("\n");
  // }

  // ============================================================================

  // Param params[] = {
  //   { .name = "not_latitude", .value = "43.6046"},
  //   { .name = "not_longitude", .value = "1.4451"},
  // };
  // Params awqat_query_params = {0};
  // for (size_t i = 0; i < NOB_ARRAY_LEN(params); i++) {
  //   printf("Name: %s\n\tValue: %s\n", params[i].name, params[i].value);
  //   nob_da_append(&awqat_query_params, params[i]);
  // }


  if (city)
    awq_get_coord_by_city(&awq_params, city, NOMINATIM_URL);
  else
    awq_get_user_coord(&awq_params, IP_API_URL);

  Nob_String_Builder resp = (Nob_String_Builder){0};
  Nob_String_Builder date = get_date_now();
  int result = awq_fetch(ALADHAN_TIMINGS_URL,
      date.items,
      &awq_params,
      &resp);

  awq_delete_params(&awq_params);

  if (result) {
    fprintf(stderr, "[awqat] Something wrong when fetching. Exiting..\n");
    exit(EXIT_FAILURE);
  }

  nob_sb_free(date);

  cJSON *return_json = cJSON_Parse(nob_sb_to_sv(resp).data);
  nob_sb_free(resp);

  int code = cJSON_GetObjectItem(return_json, "code")->valueint;

  if (code == 400) {
    fprintf(stderr, "[awqat] aladhan error %d: %s\n", code,
        cJSON_GetObjectItem(return_json, "data")->valuestring);
    exit(EXIT_FAILURE);
  }

  cJSON *timings = awq_json_get_nested(return_json, "data.timings");
  const char *method = awq_json_get_nested(return_json, "data.meta.method.name")->valuestring;

  const char *prayer_names[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};
  const char *prayer_times[NOB_ARRAY_LEN(prayer_names)];
  int diff[NOB_ARRAY_LEN(prayer_times)];

  printf("%-8s: %s\n", "Method", method);
  Nob_String_Builder now = get_time_now();
  for (size_t i = 0; i < NOB_ARRAY_LEN(prayer_names); i++) {
    prayer_times[i] = cJSON_GetObjectItem(timings, prayer_names[i])->valuestring;
    diff[i] = parse_minutes(prayer_times[i]) - parse_minutes(now.items);
  }
  nob_sb_free(now);

  int min = INT_MAX;
  size_t min_idx = 0;

  for (size_t i = 0; i < NOB_ARRAY_LEN(prayer_names); i++) {
    if (diff[i] > 0 && diff[i] < min) {
      min = diff[i];
      min_idx = i;
    }
  }

  for (size_t i = 0; i < NOB_ARRAY_LEN(prayer_names); i++) {
    if (i == min_idx)
      printf(BOLD RED "%-8s: %s%s" RESET "\n", prayer_names[i], prayer_times[i], " *");
    else
      printf("%-8s: %s\n", prayer_names[i], prayer_times[i]);
  }

  cJSON_Delete(return_json);

  return 0;
}
