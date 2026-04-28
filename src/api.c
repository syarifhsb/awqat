#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "api.h"

#include "../nob.h"

size_t fetch_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  Nob_String_Builder *resp = (Nob_String_Builder *)userdata;
  size_t real_size = size * nmemb;

  nob_sb_append_cstr(resp, ptr);
  nob_sb_append_null(resp);

  return real_size; // must return real_size or curl aborts
}

Nob_String_Builder awq_build_url(const char *main_url, const char *path_params, const Params *query_params) {
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
      nob_sb_append_sv(&url_sb, nob_sb_to_sv(query_params->items[i].name));
      nob_sb_append(&url_sb, '=');
      nob_sb_append_sv(&url_sb, nob_sb_to_sv(query_params->items[i].value));
      nob_sb_append(&url_sb, '&');
    }
  }

  nob_sb_append_null(&url_sb);

  return url_sb;
}

int awq_fetch(const char *main_url, const char *path_params, const Params *query_params, Nob_String_Builder *response) {
  Nob_String_Builder url_sb = awq_build_url(main_url, path_params, query_params);

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
  curl_slist_append(headers, "User-Agent: awqat-cli/1.0");

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fetch_write_callback);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  CURLcode result = curl_easy_perform(curl);

  nob_sb_free(url_sb);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();

  return (int)result;
}

int awq_add_param(Params *params, const char *name, const char *value) {
  Param p = {0};
  nob_sb_append_cstr(&p.name, name);
  nob_sb_append_cstr(&p.value, value);
  nob_da_append(params, p);

  return 0;
}

int awq_delete_params(Params *params) {
  for (size_t i = 0; i < params->count; i++) {
    nob_sb_free(params->items[i].name);
    nob_sb_free(params->items[i].value);
  }
  nob_da_free(*params);

  return 0;
}
