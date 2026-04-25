#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <time.h>
#include "api.h"
#include "../nob.h"
#include "../config.h"

struct Coordinate {
  double lat;
  double lon;
};

const char* ip_url_builder() {
  static char buf[1024];
  snprintf(buf, sizeof(buf), "%s?%s", ip_api_url, "fields=status,message,lat,lon");
  return buf;
}

size_t ip_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  Response *resp = (Response *)userdata;
  size_t real_size = size * nmemb;

  resp->data = realloc(resp->data, resp->len + real_size + 1);
  memcpy(resp->data + resp->len, ptr, real_size);
  resp->len += real_size;
  resp->data[resp->len] = '\0';

  return real_size; // must return real_size or curl aborts
}

int user_coord_get() {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    return 1;
  }

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_URL, ip_url_builder());

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept-Encoding: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ip_callback);

  Response resp = {0};
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);

  CURLcode result = curl_easy_perform(curl);

  if (result == CURLE_OK) {

    char *ct;
    /* ask for the content-type */
    result = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);

    if((result == CURLE_OK) && ct)
      printf("We received Content-Type: %s\n", ct);
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();

  return (int)result;
}

// Specific gregorian date in DD-MM-YYYY format
const char* get_date_now() {
  static char buf[64];
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  snprintf(buf, sizeof(buf), "%02d-%02d-%04d", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
  return buf;
}

const char* aladhan_url_build() {
  static char buf[1024];
  snprintf(buf, sizeof(buf), "%s/%s%s", mainUrl, get_date_now(), restUrl);
  return buf;
}


size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  Response *resp = (Response *)userdata;
  size_t real_size = size * nmemb;

  resp->data = realloc(resp->data, resp->len + real_size + 1);
  memcpy(resp->data + resp->len, ptr, real_size);
  resp->len += real_size;
  resp->data[resp->len] = '\0';

  return real_size; // must return real_size or curl aborts
}

int api_curl_get(const char *url, void *cb, Response *response) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    return 1;
  }

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_URL, url);

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept-Encoding: ");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

  CURLcode result = curl_easy_perform(curl);

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();

  return (int)result;
}
