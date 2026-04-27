#ifndef AWQAT_UTILS_H_
#define AWQAT_UTILS_H_

#include "../nob.h"

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

Nob_String_Builder get_date_now();
Nob_String_Builder get_time_now();
int parse_minutes(const char *time);

#endif // AWQAT_UTILS_H_
