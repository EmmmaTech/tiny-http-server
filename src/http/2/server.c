#include "server.h"

#include <stdio.h>
#include <stdint.h>
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

#include "io/tcp.h"
#include "macros.h"
#include "srvfiles.h"
#include "types/io.h"

#define INIT_BUFFER_SIZE 65535

SSL_CTX* sslctx;

static const unsigned char alpn_test[] = {'h', '2'};
static const char* client_preface = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";

// copied from https://github.com/openssl/openssl/blob/7bdc0d13d2b9ce1c1d0ec1f89dacc16e5d045314/demos/http3/ossl-nghttp3-demo-server.c#L892-L902
static int select_alpn(SSL *ssl, const unsigned char **out,
                       unsigned char *out_len, const unsigned char *in,
                       unsigned int in_len, void *arg)
{
    if (SSL_select_next_proto((unsigned char **)out, out_len, alpn_test,
                              sizeof(alpn_test), in,
                              in_len) != OPENSSL_NPN_NEGOTIATED)
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

void* handle_stream(void* obj);

void http_2_server_loop(in_addr_t address, int port)
{
    initialize_tls();
    open_server(address, port);
    connection_loop(handle_stream);
}

void* handle_stream(void* obj)
{
    int handle = *((int*) obj);
    free(obj);

    unsigned char* buffer;
    buffer = calloc(INIT_BUFFER_SIZE, sizeof(unsigned char));

    SSL* cssl = SSL_new(sslctx);
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

    bzero(buffer, sizeof(buffer));

    int readlen = SSL_read(cssl, buffer, sizeof(buffer));
    if (!STRINGS_EQUAL_N(buffer, client_preface, strlen(client_preface)))
    {
        // TODO: raise error here
    }
}
