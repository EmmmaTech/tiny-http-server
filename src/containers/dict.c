#include "dict.h"

#include <stdlib.h>
#include <string.h>

dict_t* dict_create() 
{
    dict_t* container = malloc(sizeof(dict_t));

    container->key = NULL;
    container->value = NULL;
    container->parent = NULL;
    container->left = NULL;
    container->right = NULL;

    return container;
}

void dict_delete(dict_t* dict) 
{
    /* TODO */
}

void dict_append(dict_t* dict, const char* key, const void* value) 
{
    if (dict->key == NULL && dict->value == NULL) 
    {
        dict->key = key;
        dict->value = value;
        return;
    }

    dict_t* node = dict_create();
    node->key = key;
    node->value = value;

    int cmp;
    dict_t* parent = dict;
    while (parent != NULL) 
    {
        cmp = strcmp(parent->key, key);
        dict_t* possible;

        if (cmp < 0) possible = parent->left;
        else if (cmp > 0) possible = parent->right;

        if (possible != NULL) parent = possible;
        else break;
    }

    node->parent = parent;

    if (cmp < 0) parent->left = node;
    else parent->right = node;
}

const void* dict_get(dict_t* dict, const char* key) 
{
    int cmp;
    dict_t* i = dict;

    while (i != NULL && i->key != NULL) 
    {
        cmp = strcmp(i->key, key);

        if (cmp == 0) return i->value;
        else if (cmp < 0) i = i->left;
        else i = i->right;
    }

    return NULL;
}
