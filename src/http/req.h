#ifndef REQ_H
#define REQ_H

#include "../containers/dict.h"
#include "../types/io.h"

#define METHOD_MAX_SIZE 8
#define PATH_MAX_SIZE 256
#define VERSION_MAX_SIZE 10

typedef struct {
    char method[METHOD_MAX_SIZE];
    char path[PATH_MAX_SIZE];
    char version[VERSION_MAX_SIZE];
    dict_t* headers;

    char* payload;
    int payload_len;
} http_req_t;

const http_req_t* extract_req_info(handles_t* handles, const char input[], int len);

#endif