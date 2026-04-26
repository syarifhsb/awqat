#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <curl/curl.h>
#include "api.h"
#include "utils.h"
#include "cJSON.h"

#include "../config.h"
#include "../nob.h"

int main(int argc, char *argv[]) {

  Params user_location = awq_get_user_coord();

  Nob_String_Builder resp = (Nob_String_Builder){0};
  Nob_String_Builder date = get_date_now();
  int result = awq_fetch(ALADHAN_TIMINGS_URL,
      get_date_now().items,
      &user_location,
      &resp);

  if (!result) {
    fprintf(stderr, "[awqat] Something wrong when fetching. Exiting..\n");
    exit(EXIT_FAILURE);
  }

  // printf("Result: %d\n", result);
  // printf("Response:\n%s\n", resp.items);
  nob_sb_free(date);

  cJSON *return_json = cJSON_Parse(nob_sb_to_sv(resp).data);
  nob_sb_free(resp);

  const char *code = cJSON_Print(cJSON_GetObjectItem(return_json, "code"));

  if (strcmp(code, "400") == 0) {
    fprintf(stderr, "[awqat] aladhan error %s: %s\n", code,
        cJSON_Print(cJSON_GetObjectItem(return_json, "data")));
  }

  cJSON_Delete(return_json);

  return 0;
}
