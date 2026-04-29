#include <curl/curl.h>
#include "api.h"
#include "cJSON.h"
#include "time.h"

#include "../nob.h"

cJSON* awq_get_prayer_times(const Params *params, const char *aladhan_api_url) {
  Nob_String_Builder resp = (Nob_String_Builder){0};
  Nob_String_Builder date = awq_get_date_now();
  int result = awq_fetch(aladhan_api_url,
      date.items,
      params,
      &resp);

  nob_sb_free(date);

  if (result) {
    fprintf(stderr, "Error: failed to fetch Aladhan API\n");
    exit(EXIT_FAILURE);
  }

  cJSON *data = cJSON_Parse(nob_sb_to_sv(resp).data);
  nob_sb_free(resp);

  int code = cJSON_GetObjectItem(data, "code")->valueint;

  if (code == 400) {
    fprintf(stderr, "Error: aladhan api error %d: %s\n", code,
        cJSON_GetObjectItem(data, "data")->valuestring);
    exit(EXIT_FAILURE);
  }

  return data;
}

int awq_get_user_coord(Params *ret_params, const char *ip_api_url) {
  Nob_String_Builder resp = {0};
  int result = awq_fetch(ip_api_url,
      NULL,
      NULL,
      &resp);

  if (result != CURLE_OK) {
      fprintf(stderr, "Error: failed to fetch IP API\n");
      exit(EXIT_FAILURE);
  }

  cJSON *data = cJSON_Parse(nob_sb_to_sv(resp).data);
  nob_sb_free(resp);

  cJSON *latitude = cJSON_GetObjectItem(data, "lat");
  cJSON *longitude = cJSON_GetObjectItem(data, "lon");

  if (!cJSON_IsNumber(latitude) || !cJSON_IsNumber(longitude)) {
      fprintf(stderr, "failed to parse IP API JSON\n");
      exit(EXIT_FAILURE);
  }

  // TODO: pass value without cJSON_Print
  char *lat_s = cJSON_Print(latitude);
  char *lon_s = cJSON_Print(longitude);

  awq_add_param(ret_params, "latitude", lat_s);
  awq_add_param(ret_params, "longitude", lon_s);

  cJSON_free(lat_s);
  cJSON_free(lon_s);

  cJSON_Delete(data);

  return 0;
}

Nob_String_Builder awq_get_coord_by_city(Params *ret_params, const char *city, const char *nominatim_url, int return_city_sb) {
  Params nominatim_params = {0};

  awq_add_param(&nominatim_params, "city", city);
  awq_add_param(&nominatim_params, "format", "json");
  awq_add_param(&nominatim_params, "limit", "1");

  Nob_String_Builder resp = {0};
  int result = awq_fetch(nominatim_url,
      "search",
      &nominatim_params,
      &resp);

  if (result != CURLE_OK) {
      fprintf(stderr, "Error: failed to fetch Geolocation API\n");
      exit(EXIT_FAILURE);
  }

  awq_delete_params(&nominatim_params);

  cJSON *return_json = cJSON_Parse(nob_sb_to_sv(resp).data);
  nob_sb_free(resp);

  cJSON *data = cJSON_GetArrayItem(return_json, 0);
  if (!data) {
    fprintf(stderr, "Error: Could not find city: %s\n", city);
    exit(EXIT_FAILURE);
  }

  cJSON *latitude = cJSON_GetObjectItem(data, "lat");
  cJSON *longitude = cJSON_GetObjectItem(data, "lon");

  awq_add_param(ret_params, "latitude", latitude->valuestring);
  awq_add_param(ret_params, "longitude", longitude->valuestring);

  Nob_String_Builder ret_sb = {0};
  if (return_city_sb) {
    cJSON *city = cJSON_GetObjectItem(data, "display_name");
    nob_sb_append_cstr(&ret_sb, city->valuestring);
    nob_sb_append_null(&ret_sb);
  }

  cJSON_Delete(return_json);

  return ret_sb;
}
