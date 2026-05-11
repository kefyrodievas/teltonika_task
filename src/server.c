#include "server.h"
#include "cJSON.h"
#include <bits/time.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/multi.h>
#include <curl/system.h>
#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "speedtest.h"
#include "netutils.h"
#include <time.h>


char * read_file(char *fname){
    struct stat file_stat;
    int stat_res = stat(fname, &file_stat);
    if(stat_res == -1){
        printf("file cannot be accessed: %s\n", fname);
        abort();
    }
    char * file_data = (char *)malloc((file_stat.st_size+1) * sizeof(char));
    FILE* in_file = fopen(fname, "r");
    if(in_file == NULL){
        printf("error opening file: %s\n", fname);
        abort();
    }
    fread(file_data, sizeof(char), file_stat.st_size, in_file);
    fclose(in_file);
    file_data[file_stat.st_size] = '\0';
    return file_data;

}

//works, do not touch
int get_country_servers(cJSON * root, struct location loc){
    int index = 0;
    int res;

    for(cJSON * elem = root->child; elem != NULL; index++){

        char * country = cJSON_GetObjectItem(elem, "country")->valuestring;
        char * host = cJSON_GetObjectItem(elem, "host")->valuestring;

        if((res = strcmp(cJSON_GetObjectItem(elem, "country")->valuestring, loc.country)) != 0){
            
            if(elem->next != NULL){
                elem = elem->next;
                cJSON_DeleteItemFromArray(root, index);
                index--;
            }
            else{
                cJSON_DeleteItemFromArray(root, index);
                break;
            }
        }
        else{
            elem = elem->next;
        }
        // printf("%d\n", res);
        
    }
    // printf("%d\n",cJSON_GetArraySize(root));
    for(cJSON * elem = root->child; elem != NULL; elem = elem->next, index++){
        char * country = cJSON_GetObjectItem(elem, "country")->valuestring;
        char * host = cJSON_GetObjectItem(elem, "host")->valuestring;

        // printf("%s, %s\n", country, host);
    }
    return cJSON_GetArraySize(root);
}

// actually horrible, poor results with sufficiently large server count
char * get_best_server(cJSON * root){
    struct memory chunk = { 0 };
    cJSON * elem;
    long mintime = LONG_MAX;
    char * best;
    CURLM * multi = curl_multi_init();
    curl_multi_setopt(multi, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
    curl_multi_setopt(multi, CURLMOPT_MAX_TOTAL_CONNECTIONS, 6000);
    char errbuf[512];
    cJSON_ArrayForEach(elem, root){
        CURL * curl;
        curl = curl_easy_init();
        char url[256] = "";
        char * host = cJSON_GetObjectItem(elem, "host")->valuestring;
        sprintf(url, "%s/speedtest/latency.txt", host);

        // setting up each individual transfer
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
        // put a long timeout in case of high server count
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
        // servers return http 500 without user agent, no idea why
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0");
        curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
        // avoid the output of transfer printing out to stdout
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_multi_add_handle(multi, curl);
    }
    int running_handles = 0;
    uint8_t still_running = 1;
    for (;;){
        CURLMcode mcode = curl_multi_perform(multi, &running_handles);
        if(mcode != CURLM_OK){
            printf("curl_multi_perform_failed\n");
            break;
        }
        if(running_handles == 0){
            break;
        }
        mcode = curl_multi_poll(multi, NULL, 0, 200, NULL);
        if(mcode != CURLM_OK) {
            fprintf(stderr, "curl_multi_poll() failed, code %d.\n", (int)mcode);
            break;
        }
    }
    CURLMsg * msg;
    int msg_in_queue = 0;
    
    while((msg = curl_multi_info_read(multi, &msg_in_queue)) != NULL){
        long code;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_HTTP_CODE, &code);
        if(msg->msg == CURLMSG_DONE && msg->data.result == CURLE_OK && code == 200){
            char * url;
            long time = 0, starttime = 0;

            // getting transfer info
            curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &url);            
            curl_easy_getinfo(msg->easy_handle, CURLINFO_CONNECT_TIME_T, &starttime);
            curl_easy_getinfo(msg->easy_handle, CURLINFO_TOTAL_TIME_T, &time);
            
            // check the current fastest time
            if(mintime > time - starttime){
                mintime = time - starttime;
                best = strdup(url);
            }
        }
        curl_easy_cleanup(msg->easy_handle);
    }
    int pos = strlen(best) - strlen("/speedtest/latency.txt");
    // cut off the latency file in the url, horrendous but works
    memset(best+pos, 0, strlen("/speedtest/latency.txt"));
    return best;
}