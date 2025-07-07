#include "req.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "resp.h"

#define CHECK_END_OF_LINE(ptr, end, len) \
    if (len >= end - ptr) { \
        http_resp_t* resp = create_error_resp(400); \
        send_resp(resp, handles); \
        return NULL; \
    }

const http_req_t* extract_req_info(handles_t* handles, const char input[], int len) 
{
    char *name, *value;
    int curlen;

    http_req_t* req = malloc(sizeof(http_req_t));
    req->headers = NULL;
    req->payload = NULL;

    const char* start = &input[0];
    const char* end = start + len;

    char* line = (char*) start;
    const char* line_end = strstr(line, "\r\n");

    curlen = strcspn(line, " ");
    CHECK_END_OF_LINE(line, line_end, curlen);
    memcpy(req->method, line, MIN(LEN(req->method), curlen));
    line += curlen;
    line += strspn(line, " ");

    curlen = strcspn(line, " ");
    CHECK_END_OF_LINE(line, line_end, curlen);
    memcpy(req->path, line, MIN(LEN(req->path), curlen));
    line += curlen;
    line += strspn(line, " ");

    curlen = line_end - line;
    memcpy(req->version, line, MIN(LEN(req->version), curlen));
    line += curlen;
    line += strspn(line, "\r\n");

    req->headers = dict_create();

    if (line != end)
    {
        for (line_end = line + strcspn(line, "\r\n"); line_end != end; line += strspn(line, "\r\n"), line_end = line + strcspn(line, "\r\n"))
        {
            int namelen = strcspn(line, ":");
            name = calloc(namelen, sizeof(char));
            memcpy(name, line, namelen);

            for (int i = 0; i < namelen; i++) name[i] = tolower(name[i]);

            line += namelen;
            line += strspn(line, ": ");

            int valuelen = strcspn(line, "\r\n");
            value = calloc(valuelen, sizeof(char));
            memcpy(value, line, valuelen);
            line += valuelen;

            dict_append(req->headers, name, value);
        }

        const char* raw = (char*) dict_get(req->headers, "content-length");
        if (raw != NULL)
        {
            req->payload_len = atoi(raw);
            if (req->payload_len > 0)
            {
                req->payload = calloc(req->payload_len, sizeof(char));

                // skip forward to actual content
                line += strspn(line, "\r\n");
                memcpy(req->payload, line, req->payload_len);
            }
        }
        else if (line + strspn(line, "\r\n") != end)
        {
            http_resp_t* resp = create_resp(411, NULL, NULL, 0);
            send_resp(resp, handles);
            return NULL;
        }
    }

    return req;
}

#undef CHECK_END_OF_LINE
