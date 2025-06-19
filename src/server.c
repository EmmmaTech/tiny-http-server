#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "handlers.h"
#include "http/req.h"
#include "http/resp.h"
#include "macros.h"
#include "srvfiles.h"

#define CONN_QUEUE_SIZE 200
#define CONN_LIMIT 400
#define THREAD_STACK_SIZE 524288
#define BUFFER_SIZE 65536

void server_loop(in_addr_t address, int port)
{
    pthread_t thread;
    pthread_attr_t thread_attrs;

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

    int sock_handle, conn_handle;
    struct sockaddr_in server, client;
    socklen_t serverlen, clientlen;
    
    memset(&server, 0, sizeof(server));
    memset(&client, 0, sizeof(client));

    sock_handle = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_handle < 0)
    {
        perror("failed to open main socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = address;
    server.sin_port = htons(port);

    if (bind(sock_handle, (struct sockaddr*)(&server), sizeof(server)))
    {
        perror("failed to bind port");
        exit(1);
    }

    serverlen = sizeof(server);
    if (getsockname(sock_handle, (struct sockaddr*)(&server), &serverlen))
    {
        perror("failed to get socket name");
        exit(1);
    }

    printf("http server: socket opened on address %s, with port %d\n", inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    if (listen(sock_handle, CONN_QUEUE_SIZE) < 0)
    {
        perror("failed to open socket for listening");
        exit(1);
    }

    while (1)
    {
        conn_handle = accept(sock_handle, (struct sockaddr*)(&client), &clientlen);
        
        if (conn_handle < 0)
        {
            perror("failed to accept connection");
            continue;
        }

        printf("http server: accepted connection from %s\n", inet_ntoa(client.sin_addr));

        int* handle_ptr = (int*)malloc(sizeof(int));
        if (handle_ptr == NULL)
        {
            fprintf(stderr, "FATAL: could not allocate memory for handle pointer\n");
            close(conn_handle);
            continue;
        }

        *handle_ptr = conn_handle;

        if (pthread_create(&thread, &thread_attrs, handle_connection, handle_ptr))
        {
            perror("could not create new thread");
            close(conn_handle);
            free(handle_ptr);
            continue;
        }
    }

    close(sock_handle);
}

int default_req_handle(int handle, const http_req_t* req)
{
    http_resp_t* resp;
    const char* file;
    const char* name;
    int status;

    if (STRINGS_EQUAL(req->method, "GET"))
    {
        if (STRINGS_EQUAL(req->path, "/"))
            name = "index.html";
        else
            name = &req->path[1];

        file = (char*) dict_get(files, name);

        if (file != NULL)
            status = 200;
        else
        {
            name = "404.html";
            file = (char*) dict_get(files, name);
            if (file == NULL)
                file = "Not Found";

            status = 404;
        }
    }
    else
    {
        name = "405.html";
        file = (char*) dict_get(files, name);
        if (file == NULL)
            file = "Method Not Allowed";
        
        status = 405;
    }

    resp = create_resp(status, name, file, strlen(file));
    send_resp(resp, handle);
    return 0;
}

void* handle_connection(void* handle_ptr)
{
    int handle = *((int*) handle_ptr);
    free(handle_ptr);

    char buffer[BUFFER_SIZE];
    bzero(buffer, sizeof(buffer));

    int readlen = read(handle, buffer, sizeof(buffer));
    if (readlen > 0)
    {
        printf("received (%d): %s\n", readlen, buffer);

        const http_req_t* req = extract_req_info(buffer, readlen);
        printf("req for %s with method %s and version %s\n", req->path, req->method, req->version);

        handler_func_t handler = get_handler(req->method, req->path);

        if (handler != NULL)
            handler(handle, req);
        else
            default_req_handle(handle, req);
    }
    else if (readlen < 0)
        perror("failed to read");

    close(handle);
    pthread_exit(NULL);
}
