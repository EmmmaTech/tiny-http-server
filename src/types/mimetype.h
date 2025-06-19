#ifndef MIMETYPE_H
#define MIMETYPE_H

#include <stdbool.h>

typedef struct {
    const char* ext;
    const char* mime;
    bool text;
} mimetype_t;

extern const mimetype_t mimetypes[];
extern const int mimetypes_size;

const mimetype_t* mimetype_from_ext(const char* ext);
const mimetype_t* mimetype_from_mime(const char* mime);

const mimetype_t* filename_to_mime(const char* filename);

#endif