#pragma once
#include "cJSON.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include "location.h"

char * read_file(char * fname);
char * get_best_server(cJSON * root);
int get_country_servers(cJSON * root, struct location loc);