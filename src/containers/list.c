#include "list.h"

#include <stdlib.h>

list_t* list_create()
{
    list_t* container = malloc(sizeof(list_t));

    container->first = NULL;
    container->size = 0;

    return container;
}

void free_node(list_node_t* node)
{
    node->next = NULL;
    free(node);
}

void list_delete(list_t* list)
{
    size_t currsize = list->size;
    for (size_t i = 0; i < currsize; i++)
    {
        list_node_t* node = list_pop(list);
        free_node(node);
    }

    list->first = NULL;
    free(list);
}

void list_append(list_t* list, const void* value)
{
    list_node_t* new = malloc(sizeof(list_node_t));
    new->value = value;
    new->prev = NULL;
    new->next = NULL;

    list->size++;

    if (list->first == NULL)
    {
        list->first = new;
        return;
    }

    list_node_t* i = list->first;
    while (i->next != NULL)
        i = i->next;

    new->prev = i;
    i->next = new;
}

list_node_t* list_pop(list_t* list)
{
    list_node_t* i = list->first;
    while (i->next != NULL)
        i = i->next;

    i->prev->next = NULL;
    list->size--;
    return i;
}
