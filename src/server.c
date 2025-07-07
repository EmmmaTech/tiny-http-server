#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "handlers.h"
#include "http/req.h"
#include "http/resp.h"
#include "macros.h"
#include "srvfiles.h"
#include "types/io.h"

#define CONN_QUEUE_SIZE 200
#define CONN_LIMIT 400
#define THREAD_STACK_SIZE 524288
#define BUFFER_SIZE 65536

// if you want http fallback, set this to 1
#define HTTP_FALLBACK 0

#define HANDLE_SSL_ERR_ARGS(status, txt, ...) \
    ERR_print_errors_fp(stderr); \
    fprintf(stderr, txt, __VA_ARGS__); \
    exit(status);

#define HANDLE_SSL_ERR(status, txt) \
    ERR_print_errors_fp(stderr); \
    fprintf(stderr, txt); \
    exit(status);

const unsigned char alpn_support[] = {8, 'h', 't', 't', 'p', '/', '1', '.', '1'};

struct _pass_to_thread {
    int strm_handle;
    bool uses_tls;
}; 

SSL_CTX* sslctx; 

void* handle_connection(void* obj);

int select_alpn(SSL *ssl, const unsigned char **out,
                       unsigned char *out_len, const unsigned char *in,
                       unsigned int in_len, void *arg)
{
    if (SSL_select_next_proto((unsigned char**)out, out_len, alpn_support, sizeof(alpn_support), in, in_len) != OPENSSL_NPN_NEGOTIATED)
        return SSL_TLSEXT_ERR_ALERT_FATAL;

    return SSL_TLSEXT_ERR_OK;
}

void initialize_tls()
{
    SSL_load_error_strings();
    SSL_library_init();

    sslctx = SSL_CTX_new(TLS_server_method());
    if (sslctx == NULL)
    {
        HANDLE_SSL_ERR(1, "failed to initialize an ssl ctx\n");
    }

    if (!SSL_CTX_set_min_proto_version(sslctx, TLS1_2_VERSION))
    {
        SSL_CTX_free(sslctx);
        HANDLE_SSL_ERR(1, "failed to set min TLS version to 1.2\n");
    }

    long opts = 0;
    opts |= SSL_OP_NO_RENEGOTIATION;
    opts |= SSL_OP_CIPHER_SERVER_PREFERENCE;
    SSL_CTX_set_options(sslctx, opts);

    if (SSL_CTX_use_certificate_chain_file(sslctx, "cert.pem") <= 0)
    {
        SSL_CTX_free(sslctx);
        HANDLE_SSL_ERR(1, "failed to load certificate chain file\n");
    }

    if (SSL_CTX_use_PrivateKey_file(sslctx, "key.pem", SSL_FILETYPE_PEM) <= 0)
    {
        SSL_CTX_free(sslctx);
        HANDLE_SSL_ERR(1, "failed to load certificate private key, possible that you're a moron?\n");
    }

    SSL_CTX_set_timeout(sslctx, 60 * 60);
    SSL_CTX_set_verify(sslctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_alpn_select_cb(sslctx, select_alpn, NULL);
}

void server_loop(in_addr_t address, int port, bool tls)
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

    if (tls)
        initialize_tls();

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

        struct _pass_to_thread* obj = (struct _pass_to_thread*) malloc(sizeof(struct _pass_to_thread));
        if (obj == NULL)
        {
            fprintf(stderr, "FATAL: could not allocate memory for obj passed to thread\n");
            close(conn_handle);
            continue;
        }

        obj->strm_handle = conn_handle;
        obj->uses_tls = tls;

        if (pthread_create(&thread, &thread_attrs, handle_connection, obj))
        {
            perror("could not create new thread");
            close(conn_handle);
            free(obj);
            continue;
        }
    }

    close(sock_handle);
}

int default_req_handle(handles_t* handles, const http_req_t* req)
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
    send_resp(resp, handles);
    return 0;
}

void* handle_connection(void* obj)
{
    char buffer[BUFFER_SIZE];

    struct _pass_to_thread* typed_obj = (struct _pass_to_thread*) (obj);

    bool tls = typed_obj->uses_tls;
    int handle = typed_obj->strm_handle;

    free(typed_obj);

    SSL* cssl = NULL;
    if (tls)
    {
        cssl = SSL_new(sslctx);
        SSL_set_fd(cssl, handle);

        int ssl_err = SSL_accept(cssl);
        if (ssl_err <= 0)
        {
            SSL_free(cssl);
            ERR_print_errors_fp(stderr);

            #if HTTP_FALLBACK
            fprintf(stderr, "failed to establish a TLS connection with client, attempting to perform plaintext\n");
            tls = false;
            #else
            fprintf(stderr, "failed to establish a TLS connection with client, closing connection\n");
            close(handle);
            pthread_exit(NULL);
            #endif
        }
    }

    bzero(buffer, sizeof(buffer));

    handles_t* handles = (handles_t*) malloc(sizeof(handles_t));
    handles->fd = handle;
    handles->ssl = cssl;

    int readlen = tls ? SSL_read(cssl, buffer, sizeof(buffer)) : read(handle, buffer, sizeof(buffer));
    if (readlen > 0)
    {
        printf("received (%d): %s\n", readlen, buffer);

        const http_req_t* req = extract_req_info(handles, buffer, readlen);

        if (req == NULL)
        {
            printf("error while parsing request, closing connection\n");

            close(handle);
            pthread_exit(NULL);
        }

        printf("request for %s with method %s and version %s\n", req->path, req->method, req->version);

        handler_func_t handler = get_handler(req->method, req->path);
        if (handler == NULL)
            handler = default_req_handle;

        handler(handles, req);
    }
    else if (readlen < 0)
        perror("failed to read");

    if (tls)
        SSL_free(cssl);

    close(handle);
    pthread_exit(NULL);
}
