#ifndef IO_TCP
#define IO_TCP

#include <stdbool.h>

#include <arpa/inet.h>

typedef void*(conn_handler_t)(void* obj);

extern int server_sock;

void open_server(in_addr_t address, int port);
void connection_loop(conn_handler_t handler);

#endif