#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct _list_node {
    const void* value;

    struct _list_node* prev;
    struct _list_node* next;
} list_node_t;

typedef struct {
    list_node_t* first;
    size_t size;
} list_t;

list_t* list_create();
void list_delete(list_t* list);

void list_append(list_t* list, const void* value);
list_node_t* list_pop(list_t* list);

#endif