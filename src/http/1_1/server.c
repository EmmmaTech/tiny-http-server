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
#include "http/1_1/req.h"
#include "http/1_1/resp.h"
#include "io/tcp.h"
#include "macros.h"
#include "srvfiles.h"
#include "types/io.h"

#define CONN_QUEUE_SIZE 200
#define CONN_LIMIT 400
#define THREAD_STACK_SIZE 524288
#define BUFFER_SIZE 65536

#define HANDLE_SSL_ERR_ARGS(status, txt, ...) \
    ERR_print_errors_fp(stderr); \
    fprintf(stderr, txt, __VA_ARGS__); \
    exit(status);

#define HANDLE_SSL_ERR(status, txt) \
    ERR_print_errors_fp(stderr); \
    fprintf(stderr, txt); \
    exit(status);

SSL_CTX* sslctx;
bool use_tls;

void* handle_connection(void* obj);

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
}

void http_11_server_loop(in_addr_t address, int port, bool tls)
{
    use_tls = tls;
    open_server(address, port);

    if (tls)
        initialize_tls();

    connection_loop(handle_connection);
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

    int handle = *((int*) obj);
    free(obj);

    SSL* cssl = NULL;
    if (use_tls)
    {
        cssl = SSL_new(sslctx);
        SSL_set_fd(cssl, handle);

        int ssl_err = SSL_accept(cssl);
        if (ssl_err <= 0)
        {
            SSL_free(cssl);
            ERR_print_errors_fp(stderr);
            fprintf(stderr, "failed to establish a TLS connection with client\n");

            close(handle);
            pthread_exit(NULL);
        }
    }

    bzero(buffer, sizeof(buffer));

    handles_t* handles = (handles_t*) malloc(sizeof(handles_t));
    handles->fd = handle;
    handles->ssl = cssl;

    int readlen = use_tls ? SSL_read(cssl, buffer, sizeof(buffer)) : read(handle, buffer, sizeof(buffer));
    if (readlen > 0)
    {
        printf("received (%d): %s\n", readlen, buffer);

        const http_req_t* req = extract_req_info(buffer, readlen);
        printf("req for %s with method %s and version %s\n", req->path, req->method, req->version);

        handler_func_t handler = get_handler(req->method, req->path);

        if (handler != NULL)
            handler(handles, req);
        else
            default_req_handle(handles, req);
    }
    else if (readlen < 0)
        perror("failed to read");

    if (use_tls)
        SSL_free(cssl);

    close(handle);
    pthread_exit(NULL);
}
