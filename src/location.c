#include "location.h"
#include "cJSON.h"
#include <curl/curl.h>

struct location get_location(){
    struct memory chunk = { 0 };
    long code;
    CURL * curl;
    curl = curl_easy_init();
    char *url = "http://ip-api.com/json";
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 15000);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0");
    struct location result = {0};
    CURLcode res = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &code);
    if(res != CURLE_OK || code != 200){
        return result;
    }
    cJSON * json = cJSON_Parse(chunk.response);    
    
    result.city = strdup(cJSON_GetObjectItem(json, "city")->valuestring);
    result.country = strdup(cJSON_GetObjectItem(json, "country")->valuestring);
    return result;
}