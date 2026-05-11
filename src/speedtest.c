#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>
#include <stdio.h>
#include "netutils.h"
#include "speedtest.h"

// TODO: cleanup
long download_test(char * url){
    int test =0;
    struct memory chunk = { 0 };
    CURL * curl;
    curl = curl_easy_init();
    char file_url[512] = "";
    sprintf(file_url, "%s/download?size=500000000000", url); // download size is reasonably big to have 15 seconds of time for the test
    curl_easy_setopt(curl, CURLOPT_URL, file_url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 15000);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 3000);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    // servers return http 500 without user agent, no idea why
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0");
    CURLcode res = curl_easy_perform(curl);
    
    long speed = 0;
    long code = 0;
    curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD_T, &speed);
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &code);
    
    curl_easy_cleanup(curl);
    if(code != 200){
        printf("download test failed, http code: %li\n", code);
        return -1;
    }
    return speed;
}

long upload_test(char * url){
    CURL * curl;
    curl = curl_easy_init();
    FILE * rand_file = fopen("/dev/urandom", "r");
    char file_url[512] = "";
    sprintf(file_url, "%s/upload", url);
    curl_easy_setopt(curl, CURLOPT_URL, file_url);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_READDATA, rand_file);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 15000);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 3000);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    // servers return http 500 without user agent, no idea why
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0");
    CURLcode res = curl_easy_perform(curl);
    
    long speed = 0;
    long code = 0;
    long size = 0;
    curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed);
    curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &size);
    curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &code);
    fclose(rand_file);
    curl_easy_cleanup(curl);
    if(code != 100){
        printf("upload test failed, http code: %li\n", code);
        return -1;
    }
    
    return speed;
}