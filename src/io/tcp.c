#include "tcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define CONN_QUEUE_SIZE 200
#define CONN_LIMIT 400
#define THREAD_STACK_SIZE 524288
#define BUFFER_SIZE 65536

int server_sock = 0;
pthread_attr_t thread_attrs;

void open_server(in_addr_t address, int port)
{
    if (pthread_attr_init(&thread_attrs))
    {
        perror("failed to initialize pthread attrs");
        exit(1);
    }

    if (pthread_attr_setstacksize(&thread_attrs, THREAD_STACK_SIZE))
    {
        perror("failed to set thread stack size");
        exit(1);
    }

    if (pthread_attr_setdetachstate(&thread_attrs, PTHREAD_CREATE_DETACHED))
    {
        perror("failed to set thread to detached");
        exit(1);
    }

    struct sockaddr_in server, client;
    socklen_t serverlen, clientlen;
    
    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("failed to open main socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = address;
    server.sin_port = htons(port);

    if (bind(server_sock, (struct sockaddr*)(&server), sizeof(server)))
    {
        perror("failed to bind port");
        exit(1);
    }

    serverlen = sizeof(server);
    if (getsockname(server_sock, (struct sockaddr*)(&server), &serverlen))
    {
        perror("failed to get socket name");
        exit(1);
    }

    printf("http server: socket opened on address %s, with port %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    if (listen(server_sock, CONN_QUEUE_SIZE) < 0)
    {
        perror("failed to open socket for listening");
        exit(1);
    }
}

void connection_loop(conn_handler_t handler)
{
    pthread_t thread;
    int conn_handle;
    struct sockaddr_in client;
    socklen_t clientlen;

    while (1)
    {
        conn_handle = accept(server_sock, (struct sockaddr*)(&client), &clientlen);
        
        if (conn_handle < 0)
        {
            perror("failed to accept connection");
            continue;
        }

        printf("http server: accepted connection from %s\n", inet_ntoa(client.sin_addr));

        int* handle_ptr = (int*) malloc(sizeof(int));
        if (handle_ptr == NULL)
        {
            fprintf(stderr, "FATAL: could not allocate memory for handle pointer\n");
            close(conn_handle);
            continue;
        }

        *handle_ptr = conn_handle;

        if (pthread_create(&thread, &thread_attrs, handler, handle_ptr))
        {
            perror("could not create new thread");
            close(conn_handle);
            free(handle_ptr);
            continue;
        }
    }

    close(server_sock);
}
