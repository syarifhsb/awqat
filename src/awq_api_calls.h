#include "api.h"
#include "cJSON.h"

#define NOB_UNSTRIP_PREFIX
#include "../nob.h"

typedef enum {
  AWQ_FETCH_ERR_CURL = 1,
  AWQ_FETCH_ERR_CITY_NOT_FOUND,
} awqaterrtype;

cJSON* awq_get_prayer_times(const Params *params, const char *aladhan_api_url);

int awq_get_user_coord_params(Params *ret_params, const char *ip_api_url);

int awq_get_user_coord(float *lat, float *lon, const char *ip_api_url);

int awq_get_location_name(float lat, float lon, const char *nominatim_url, Nob_String_Builder *location_name);

int awq_get_coord_by_city(float *lat, float *lon, const char *city, const char *nominatim_url, Nob_String_Builder *return_city_sb);
