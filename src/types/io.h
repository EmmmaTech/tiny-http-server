#ifndef IO_H
#define IO_H

#include <openssl/ssl.h>

typedef struct {
    int fd;
    SSL* ssl;
} handles_t;

#endif