#ifndef SERVER_H
#define SERVER_H

#include <stdbool.h>

#include <arpa/inet.h>

void server_loop(in_addr_t address, int port, bool tls);

#endif