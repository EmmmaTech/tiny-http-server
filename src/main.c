#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "handlers.h"
#include "http/req.h"
#include "http/resp.h"
#include "macros.h"
#include "server.h"
#include "srvfiles.h"

#define DEFAULT_PORT 8080

HANDLER_FUNC(user_agent)
{
    const char* user_agent = (char*) dict_get(req->headers, "User-Agent");

    if (user_agent == NULL)
    {
        http_resp_t* resp = create_resp(500, NULL, "", 0);
        send_resp(resp, handle);
        return 0;
    }

    const char* template = "<!DOCTYPE html>\n<html>\n<head>\n<title>User Agent!</title>\n</head>\n<body>\n<h1>your user agent is: %s</h1>\n</body>\n</html>";
    int size = strlen(template) + strlen(user_agent);

    char* final = calloc(size, sizeof(char));
    sprintf(final, template, user_agent);

    http_resp_t* resp = create_resp(200, "user_agent.html", final, size);
    send_resp(resp, handle);
    free(final);
    return 0;
}
HANDLER(user_agent, "GET", "/user-agent")

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: ./httpsrv servedir [port] [addr]\n");
    }

    ADD_HANDLER(user_agent);

    load_files_from(argv[1]);

    int port;
    if (argc == 3)
        port = atoi(argv[2]);
    else
        port = DEFAULT_PORT;

    in_addr_t addr;
    if (argc == 4)
        addr = inet_addr(argv[3]);
    else
        addr = INADDR_ANY;

    server_loop(addr, port);
}

/*
void handle_sigint(int sig)
{
    close(msgsock_handle);
    close(sock_handle);
    exit(0);
}

int main(int argc, char** argv)
{
    int readlen;
    char read_buff[2048];
    struct sockaddr_in msgname;
    socklen_t msgnamelen;

    if (argc < 2) {
        printf("usage: ./http servedir [port]\n");
        exit(1);
    }

    signal(SIGINT, handle_sigint);

    // TODO: load files from dir

    int port;
    if (argc == 3)
        port = atoi(argv[2]);
    else
        port = DEFAULT_PORT;

    initialize_socket(port);

    listen(sock_handle, 5);
    printf("http server: listening for connections\n");
    while (1)
    {
        msgsock_handle = accept(sock_handle, (struct sockaddr*)(&msgname), &msgnamelen);

        if (msgsock_handle == -1)
        {
            perror("failed to accept connection");
            continue;
        }

        printf("http server: accepted connection from %s\n", inet_ntoa(msgname.sin_addr));

        bzero(read_buff, sizeof(read_buff));
        readlen = read(msgsock_handle, read_buff, sizeof(read_buff));

        if (readlen > 0) 
        {
            printf("received (%d): %s\n", readlen, read_buff);

            const http_req_t* req = extract_req_info(read_buff, readlen);
            printf("req for %s with method %s and version %s\n", req->path, req->method, req->version);

            handler_func_t handler = get_handler(req->method, req->path);
            int ret = 1;
            if (handler != NULL)
                ret = handler(req);

            if (ret)
            {
                const char* document = "<!DOCTYPE html>\n<html>\n <head>\n    </head>\n   <body>\n        <h1>hai</h1>\n  </body>\n</html>";
                const http_resp_t* resp = create_resp(200, "blank.html", document, strlen(document));
                send_resp(resp);
            }
        }
        else if (readlen < 0)
            perror("failed to read");

        close(msgsock_handle);
    }

    destroy_socket();
    return 0;
}
*/
