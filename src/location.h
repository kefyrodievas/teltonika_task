#pragma once
#include <curl/curl.h>
#include "netutils.h"
#include "cJSON.h"

struct location{
    char * country;
    char * city;
};

struct location get_location();