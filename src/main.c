#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "server.h"
#include "speedtest.h"
#include "location.h"
#include "cJSON.h"

#define FLAG_AUTO 1
#define FLAG_DOWN 2
#define FLAG_UP 4
#define FLAG_LOCATION 8
#define FLAG_SERVER_FILE 16
#define FLAG_SERVER_HOST 32

int main(int argc, char* argv[]){
    CURLcode result = curl_global_init(CURL_GLOBAL_ALL);
    if(result != CURLE_OK)
        return (int)result;
    int opt;
    struct location loc;
    uint8_t flags = 0;
    char * fname;
    char * host;
    if(argc == 1){
        printf("usage: speedtest [options] \n\
                    -a: performs an automated test, use with -f\n\
                    -f <filename>: uses a server hosts JSON file to search for the best server. Is -a flag is not present, use with -l\n\
                    -l: gets user's location data\n\
                    -s <server host>: uses a specified host to perform tests, invalid if -a or -fl is used\n\
                    -d: performs a download speed test. Use with -s or -fl.\n\
                    -u: performs an upload speed test. Use with -s or -fl.\n\
                    -h: outputs options to use\n");
        return 0;
    }
    while((opt = getopt(argc, argv, ":f:s:adhlu")) != -1){
        switch(opt){
            case 'a':
                // printf("automatic test\n");
                flags |= FLAG_AUTO;
                break;
            case 'd':
                // printf("download test\n");
                flags |= FLAG_DOWN;
                break;
            case 'f':
                // printf("server file flag, fname: %s\n", optarg);
                flags |= FLAG_SERVER_FILE;
                fname = optarg;
                // printf("f");
                break;
            case 'l':
                // printf("location info\n");
                flags |= FLAG_LOCATION;
                break;
            case 's':
                // printf("server host: %s\n", optarg);
                flags |= FLAG_SERVER_HOST;
                host = optarg;
                // printf("s");
                break;
            case 'u':
                // printf("upload test\n");
                flags |= FLAG_UP;
                break;
            case ':': 
                printf("option needs a value: %c\n", optopt); 
                break; 
            case '?':
                printf("unknown option: %c\n", optopt);
            case 'h':
                printf("usage: speedtest [options] \n\
                    -a: performs an automated test, use with -f\n\
                    -f <filename>: uses a server hosts JSON file to search for the best server. Is -a flag is not present, use with -l\n\
                    -l: gets user's location data\n\
                    -s <server host>: uses a specified host to perform tests, invalid if -a or -fl is used\n\
                    -d: performs a download speed test. Use with -s or -fl.\n\
                    -u: performs an upload speed test. Use with -s or -fl.\n\
                    -h: outputs options to use\n");
                return 0;
                
        }
    }

    if(flags & FLAG_AUTO && flags & FLAG_SERVER_FILE){
        printf("Getting user location\n");
        loc = get_location();
        if(loc.country == NULL){
            printf("Could not access location API\n");
            exit(-1);
        }
        printf("Finding best server\n");
        char * file_contents = read_file(fname);
        cJSON * root = cJSON_Parse(file_contents);
        if(root == NULL){
            printf("Error parsing JSON file: %s\n", fname);
            exit(-1);
        }
        int count = cJSON_GetArraySize(root);
        cJSON * copy = cJSON_Duplicate(root, 1);
        int srv_count = get_country_servers(copy, loc);
        host = get_best_server(srv_count != 0 ? copy : root);
        printf("Performing download test\n");
        long downresult = download_test(host);
        printf("Performing upload test\n");
        long upresult = download_test(host);
        if(downresult != -1) printf("download speed: %li Mb/s\n", downresult * 8 / 1000000);
        if(upresult != -1) printf("upload speed: %li Mb/s\n", upresult * 8 / 1000000);
        printf("test server: %s\n", host);
        printf("country: %s\n", loc.country);
        return 0;
    }
    else if(flags & FLAG_AUTO && flags ^ FLAG_SERVER_FILE){
        printf("Cannot perform automatic test without servers file. See usage: speedtest -h\n");
        return -1;
    }

    // if(flags & FLAG_SERVER_FILE && flags ^ FLAG_LOCATION){
    //     printf("Not enough options")
    // }
    
    if(flags & FLAG_LOCATION){
        loc = get_location();
        if(loc.country == NULL){
            printf("Could not access location API\n");
            exit(-1);
        }
        printf("country: %s\n", loc.country);
    }
    if(flags & FLAG_SERVER_FILE && flags & FLAG_LOCATION){
        char * file_contents = read_file(fname);
        cJSON * root = cJSON_Parse(file_contents);
        if(root == NULL){
            printf("Error parsing JSON file: %s\n", fname);
            exit(-1);
        }
        int count = cJSON_GetArraySize(root);
        cJSON * copy = cJSON_Duplicate(root, 1);
        int srv_count = get_country_servers(copy, loc);
        host = get_best_server(srv_count != 0 ? copy : root);
        printf("best server: %s\n", host);
        flags |= FLAG_SERVER_HOST;
    }
    else if(flags & FLAG_SERVER_FILE && flags ^ FLAG_LOCATION){
        printf("Cannot find best server without location. See usage: speedtest -h\n");
        return -1;
    }
    
    // printf("%s, %s\n", loc.city, loc.country);
    if(flags & FLAG_DOWN && flags & FLAG_SERVER_HOST){
        long result = download_test(host);
        if(result != -1) printf("download speed: %li\n", result);
    }
    else if(flags & FLAG_DOWN && flags ^ FLAG_SERVER_HOST){
        printf("Cannot perform a speedtest without a host. See usage: speedtest -h\n");
    }
    if(flags & FLAG_UP && flags & FLAG_SERVER_HOST){
        long result = upload_test(host);
        if(result != -1) printf("upload speed:   %li\n", result);
    }
    else if(flags & FLAG_UP && flags ^ FLAG_SERVER_HOST){
        printf("Cannot perform a speedtest without a host. See usage: speedtest -h\n");
    }

    
}