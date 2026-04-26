#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "api.h"
#include "cJSON.h"

#include "../nob.h"
#include "../config.h"

Nob_String_Builder url_build(const char *main_url, const char *path_params, const Params *query_params) {
  Nob_String_Builder url_sb = {0};

  nob_sb_append_cstr(&url_sb, main_url);
  nob_sb_append(&url_sb, '/');

  // PATH PARAM
  if (path_params)
    nob_sb_append_cstr(&url_sb, path_params);

  // QUERY PARAMS
  if (query_params) {
    nob_sb_append(&url_sb, '?');
    for (size_t i = 0; i < query_params->count; i++) {
      nob_sb_append_cstr(&url_sb, query_params->items[i].name);
      nob_sb_append(&url_sb, '=');
      nob_sb_append_cstr(&url_sb, query_params->items[i].value);
      nob_sb_append(&url_sb, '&');
    }
  }

  nob_sb_append_null(&url_sb);

  return url_sb;
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  Nob_String_Builder *resp = (Nob_String_Builder *)userdata;
  size_t real_size = size * nmemb;

  nob_sb_append_cstr(resp, ptr);
  nob_sb_append_null(resp);

  return real_size; // must return real_size or curl aborts
}

int api_curl_get(const char *main_url, const char *path_params, const Params *query_params, Nob_String_Builder *response) {
  Nob_String_Builder url_sb = url_build(main_url, path_params, query_params);

  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    return 1;
  }

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_URL, url_sb.items);

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept-Encoding: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  CURLcode result = curl_easy_perform(curl);

  nob_sb_free(url_sb);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();

  return (int)result;
}

int awq_add_param(Params *params, const char *param_name, const char *param_value) {
  Param p = { .name = param_name, .value = param_value };
  nob_da_append(params, p);

  return 0;
}

Params get_user_coord() {
  Params params = {0};

  Nob_String_Builder resp = {0};
  int result = api_curl_get(IP_API_URL,
      NULL,
      NULL,
      &resp);

  if (result != CURLE_OK) {
      fprintf(stderr, "failed to fetch IP API\n");
      exit(EXIT_FAILURE);
  }

  cJSON *data = cJSON_Parse(nob_sb_to_sv(resp).data);
  nob_sb_free(resp);

  cJSON *latitude = cJSON_GetObjectItem (data, "lat");
  cJSON *longitude = cJSON_GetObjectItem (data, "lon");

  if (!cJSON_IsNumber(latitude) || !cJSON_IsNumber(longitude)) {
      fprintf(stderr, "failed to parse IP API JSON\n");
      exit(EXIT_FAILURE);
  }

  awq_add_param(&params, "latitude", cJSON_Print(latitude));
  awq_add_param(&params, "longitude", cJSON_Print(longitude));

  return params;
}
