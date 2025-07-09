#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#include <arpa/inet.h>

typedef struct {
    struct in_addr addr;
    in_port_t port;

    const char version_to_use[4];
    bool tls;
    bool http_fallback;
    uint64_t buffer_size;
    uint32_t connection_limit;
    const char cert_filename[256];
    const char privkey_filename[256];
} config_t;

extern config_t* config;

int parse_config(const char* filename);

#endif