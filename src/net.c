#include "net.h"

#include <stdio.h>
#include <stdlib.h>

struct sockaddr_in name;
socklen_t namelen;

void initialize_socket(int port)
{
    sock_handle = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_handle < 0)
    {
        perror("failed to open socket");
        exit(1);
    }

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY; // TODO: modularity of supported in addresses
    name.sin_port = htons(port);

    if (bind(sock_handle, (struct sockaddr*)(&name), sizeof(name)))
    {
        perror("failed to bind port");
        exit(1);
    }

    namelen = sizeof(name);
    if (getsockname(sock_handle, (struct sockaddr*)(&name), &namelen))
    {
        perror("failed to get socket name");
        exit(1);
    }

    printf("http server: socket opened with port %d\n", ntohs(name.sin_port));
}

void destroy_socket()
{
    close(sock_handle);
}
