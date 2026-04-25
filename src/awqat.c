#include <stdio.h>
#include <curl/curl.h>
#include "api.h"
#include "../config.h"

const char* ip_url_builder();
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
int user_coord_get();
int api_curl_get(const char *url, void *cb, Response *response);

int main() {
  printf("url: %s\n", ip_url_builder());

  Response resp = {0};
  api_curl_get(url, write_callback, &resp);

  return 0;
}
