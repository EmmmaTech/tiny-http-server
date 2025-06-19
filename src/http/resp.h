#ifndef RESP_H
#define RESP_H

#include "../containers/dict.h"
#include "../containers/list.h"
#include "../types/mimetype.h"
#include "../types/status.h"

typedef struct {
    const http_status_t* status;
    const mimetype_t* content_type;
    list_t* headers;
    const char* payload;
    int payload_len;
} http_resp_t;

http_resp_t* create_resp(int status_code, const char* filename, const char* file, int filelen);
http_resp_t* create_error_resp(int status_code);

void send_resp(const http_resp_t* resp, int handle);

#endif