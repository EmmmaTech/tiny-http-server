/* unused, kept for historical reasons (for now) */

#ifndef NET_H
#define NET_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int sock_handle, msgsock_handle;

void initialize_socket(int port);
void destroy_socket();

#endif