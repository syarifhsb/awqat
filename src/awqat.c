#include <stdio.h>
#include <curl/curl.h>
#include "api.h"
#include "utils.h"

#include "../config.h"
#include "../nob.h"

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
int user_coord_get();

int main() {
  Param params[] = {
    { .name = "latitude", .value = "43.6046"},
    { .name = "longitude", .value = "1.4451"},
  };

  Params awqat_query_params = {0};
  for (size_t i = 0; i < NOB_ARRAY_LEN(params); i++) {
    printf("Name: %s\n\tValue: %s\n", params[i].name, params[i].value);
    nob_da_append(&awqat_query_params, params[i]);
  }

  Nob_String_Builder resp = {0};
  int result = api_curl_get(timings_url, 
      write_callback, 
      get_date_now().items,
      &awqat_query_params, 
      &resp);
  printf("Result: %d\n", result);
  printf("Response: %s\n", resp.items);

  return 0;
}
