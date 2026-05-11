#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct memory {
  char *response;
  size_t size;
};

size_t read_cb(char *data, size_t size, size_t nmemb, void *clientp);