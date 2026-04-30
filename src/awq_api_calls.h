#include "api.h"
#include "cJSON.h"

#define NOB_UNSTRIP_PREFIX
#include "../nob.h"

cJSON* awq_get_prayer_times(const Params *params, const char *aladhan_api_url);

int awq_get_user_coord_params(Params *ret_params, const char *ip_api_url);

int awq_get_user_coord(float *lat, float *lon, const char *ip_api_url);

int awq_get_location_name(float lat, float lon, const char *nominatim_url, Nob_String_Builder *location_name);

// set return_city_sb to 1 if city name shall be returned as SB
Nob_String_Builder awq_get_coord_by_city(Params *ret_params, const char *city, const char *nominatim_url, int return_city_sb);
