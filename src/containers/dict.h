#ifndef DICT_H
#define DICT_H

typedef struct _dict {
    const char* key;
    const void* value;
    
    struct _dict* parent;
    struct _dict* left;
    struct _dict* right;
} dict_t;

dict_t* dict_create();
void dict_delete(dict_t* dict);

void dict_append(dict_t* dict, const char* key, const void* value);
const void* dict_get(dict_t* dict, const char* key);

#endif