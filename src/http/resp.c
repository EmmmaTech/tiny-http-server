#include "resp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "../macros.h"
#include "../srvfiles.h"

http_resp_t* create_resp(int status_code, const char* filename, const char* file, int filelen)
{
    http_resp_t* resp = malloc(sizeof(http_resp_t));

    resp->status = status_from_code(status_code);

    if (filename != NULL)
        resp->content_type = filename_to_mime(filename);
    else
        resp->content_type = NULL;

    resp->headers = NULL;
    resp->payload = file;
    resp->payload_len = filelen;

    return resp;
}

http_resp_t* create_error_resp(int status_code)
{
    char name[16];
    sprintf(name, "%d.html", status_code);

    const char* file = (char*) dict_get(files, name);
    if (file == NULL)
        file = "Error HTML Not Found";

    return create_resp(status_code, name, file, strlen(file));
}

#define ADD_CONST_HEADER(dest, buff, text) \
    sprintf(buff, text "\r\n"); \
    strcat(dest, buff);

#define ADD_HEADER(dest, buff, temp, ...) \
    sprintf(buff, temp "\r\n", __VA_ARGS__); \
    strcat(dest, buff);

void send_resp(const http_resp_t* resp, int handle)
{
    char header[2048], buff[512];
    char *final;

    sprintf(buff, "HTTP/1.1 %s\r\n", resp->status->full);
    strcpy(header, buff);

    if (resp->headers != NULL)
    {
        list_node_t* i;
        for (i = resp->headers->first; i != NULL; i = i->next)
        {
            const char* val = (char*) i->value;
            ADD_HEADER(header, buff, "%s", val);
        }
    }

    if (resp->payload_len > 0 && resp->content_type != NULL)
    {
        ADD_HEADER(header, buff, "Content-Type: %s", resp->content_type->mime);
        ADD_HEADER(header, buff, "Content-Length: %d", resp->payload_len);
    }
    ADD_CONST_HEADER(header, buff, "Server: sillygirl tech");
    ADD_CONST_HEADER(header, buff, "Connection: close"); // TODO: leave this to route handlers to allow for websockets
    strcat(header, "\r\n");

    int len = strlen(header) + resp->payload_len;
    final = calloc(len, sizeof(char));

    strcpy(final, header);
    strcat(final, resp->payload);

    write(handle, final, len);
    printf("sent (%d): %s\n", len, final);
}
