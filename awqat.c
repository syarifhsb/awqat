#include <stdio.h>
#include <curl/curl.h>


int main()
{
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();
  if (!curl) {
    curl_global_cleanup();
    return 1;
  }

  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
  curl_easy_setopt(curl, CURLOPT_URL, "https://api.aladhan.com/v1/timings/01-01-2025?latitude=51.5194682&longitude=-0.1360365&method=3&shafaq=general&tune=5%2C3%2C5%2C7%2C9%2C-1%2C0%2C8%2C-6&school=0&midnightMode=0&timezonestring=UTC&latitudeAdjustmentMethod=1&calendarMethod=UAQ&iso8601=false");

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept-Encoding: ");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();

  return (int)res;
}
