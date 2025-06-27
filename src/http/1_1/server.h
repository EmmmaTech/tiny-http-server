#ifndef HTTP_11_SERVER_H
#define HTTP_11_SERVER_H

#include <stdbool.h>

#include <arpa/inet.h>

void http_11_server_loop(in_addr_t address, int port, bool tls);

#endif