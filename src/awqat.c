#include <stdio.h>
#include <curl/curl.h>
#include "api.h"
#include "utils.h"

#include "../config.h"
#include "../nob.h"

int main() {
  Params user_location = get_user_coord();

  Nob_String_Builder resp = (Nob_String_Builder){0};
  Nob_String_Builder date = get_date_now();
  int result = api_curl_get(TIMINGS_URL,
      get_date_now().items,
      &user_location,
      &resp);
  printf("Result: %d\n", result);
  printf("Response:\n%s\n", resp.items);
  nob_sb_free(date);
  nob_sb_free(resp);

  return 0;
}
