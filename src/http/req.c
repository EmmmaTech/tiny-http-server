#include "req.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"

const http_req_t* extract_req_info(const char input[], int len) 
{
    char *name, *value;
    int curlen, i;

    http_req_t* req = malloc(sizeof(http_req_t));
    req->headers = NULL;
    req->payload = NULL;

    const char* line = &input[0];

    curlen = strcspn(line, " ");
    SCUFFED_MEMCPY(req->method, line, curlen);

    line++;
    curlen = strcspn(line, " ");
    SCUFFED_MEMCPY(req->path, line, curlen);

    line++;
    curlen = strcspn(line, "\r\n");
    SCUFFED_MEMCPY(req->version, line, curlen);

    req->headers = dict_create();
    if (*(line) == '\r')
    {
        while (1)
        {
            line += strcspn(line, "\r\n") + 2;

            if (*(line) == '\r') break;

            int namelen = strcspn(line, ":");
            name = calloc(namelen, sizeof(char));
            SCUFFED_MEMCPY(name, line, namelen);

            // second check, just in case
            if (name[0] == '\r') break;

            for (size_t j = 0; j < strlen(name); ++j) name[j] = tolower(name[j]);

            line += 2;

            int vallen = strcspn(line, "\r\n");
            value = calloc(vallen, sizeof(char));
            SCUFFED_MEMCPY(value, line, vallen);

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
                SCUFFED_MEMCPY(req->payload, line, req->payload_len);
            }
        }
    }

    return req;
}
