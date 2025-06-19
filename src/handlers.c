#include "handlers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "containers/list.h"
#include "macros.h"

dict_t* handlers = NULL;

void add_handler(const handler_t* handler)
{
    if (handlers == NULL)
        handlers = dict_create();

    list_t* item = (list_t*) dict_get(handlers, handler->path);

    if (item == NULL)
    {
        item = list_create();
        dict_append(handlers, handler->path, item);
    }

    list_append(item, handler);
}

handler_func_t get_handler(const char method[METHOD_MAX_SIZE], const char path[PATH_MAX_SIZE])
{
    if (handlers == NULL)
        return NULL;

    list_t* item = (list_t*) dict_get(handlers, path);
    if (item == NULL)
        return NULL;

    list_node_t* i;
    const handler_t* val;

    for (i = item->first; i != NULL; i = i->next)
    {
        val = (handler_t*)i->value;

        if (STRINGS_EQUAL(val->method, method))
            return val->handler;
    }

    return NULL;
}
