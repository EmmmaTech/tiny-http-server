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
