#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "api.h"
#include "../nob.h"

Nob_String_Builder url_build(const char *main_url, const char *path_params, Params *query_params) {
  Nob_String_Builder url_sb = {0};

  nob_sb_append_cstr(&url_sb, main_url);
  nob_sb_append(&url_sb, '/');
  // PATH PARAM
  nob_sb_append_cstr(&url_sb, path_params);

  // QUERY PARAMS
  nob_sb_append(&url_sb, '?');
  for (size_t i = 0; i < query_params->count; i++) {
    nob_sb_append_cstr(&url_sb, query_params->items[i].name);
    nob_sb_append(&url_sb, '=');
    nob_sb_append_cstr(&url_sb, query_params->items[i].value);
    nob_sb_append(&url_sb, '&');
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

int api_curl_get(const char *main_url, void *cb, const char *path_params, Params *query_params, Nob_String_Builder *response) {
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
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  CURLcode result = curl_easy_perform(curl);

  nob_sb_free(url_sb);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();

  return (int)result;
}
