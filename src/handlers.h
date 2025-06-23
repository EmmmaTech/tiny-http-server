#ifndef HANDLERS_H
#define HANDLERS_H

#include "containers/dict.h"
#include "http/req.h"
#include "types/io.h"

typedef int (*handler_func_t)(handles_t* handles, const http_req_t*);

typedef struct {
    const char method[METHOD_MAX_SIZE];
    const char path[PATH_MAX_SIZE];
    handler_func_t handler;
} handler_t;

#define HANDLER_FUNC(name) int name ## _handler_func(handles_t *handles, const http_req_t* req)
#define HANDLER(name, method, path) const handler_t name ## _handler = { method, path, name ## _handler_func };
#define ADD_HANDLER(name) add_handler(&name ## _handler);

extern dict_t* handlers;

void add_handler(const handler_t* handler);
handler_func_t get_handler(const char method[METHOD_MAX_SIZE], const char path[PATH_MAX_SIZE]);

#endif