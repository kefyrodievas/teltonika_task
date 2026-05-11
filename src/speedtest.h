#pragma once
#include <stdint.h>
#include <stdlib.h>


long download_test(char * url);
size_t read_cb(char *data, size_t size, size_t nmemb, void *clientp);
long upload_test(char * url);