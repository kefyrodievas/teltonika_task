#include "netutils.h"

size_t read_cb(char *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = nmemb;
    struct memory *mem = (struct memory *)clientp;
    
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(!ptr) return 0;  /* out of memory */
    // printf("%li\n", mem->size);
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
    
    return realsize;
}