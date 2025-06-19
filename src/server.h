#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>

void server_loop(in_addr_t address, int port);
void* handle_connection(void* handle_ptr);

#endif