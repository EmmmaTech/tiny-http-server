#include "mimetype.h"

#include <stdlib.h>
#include <string.h>

#include "../macros.h"

#define MIMETYPE(ext, mime, text) { ext, mime, text }
const mimetype_t mimetypes[] = {
    MIMETYPE("", "application/octet-stream", false),

    MIMETYPE("json", "application/json", true),

    MIMETYPE("mp3", "audio/mpeg", false),
    MIMETYPE("ogg", "audio/ogg", false),

    MIMETYPE("jpg", "image/jpeg", false),
    MIMETYPE("jpeg", "image/jpeg", false),
    MIMETYPE("png", "image/png", false),
    MIMETYPE("svg", "image/svg+xml", true),

    MIMETYPE("css", "text/css", true),
    MIMETYPE("html", "text/html", true),
    MIMETYPE("htm", "text/html", true),
    MIMETYPE("js", "text/javascript", true),
    MIMETYPE("txt", "text/plain", true),
    MIMETYPE("xml", "text/xml", true), // CONSIDERATION: xml can also be apart of the application mimetype group like json
};
const int mimetypes_size = sizeof(mimetypes) / sizeof(mimetypes[0]);
#undef MIMETYPE

const mimetype_t* mimetype_from_ext(const char* ext)
{
    for (int i = 0; i < mimetypes_size; i++)
        if (STRINGS_EQUAL(mimetypes[i].ext, ext))
            return &mimetypes[i];

    return &mimetypes[0];
}

const mimetype_t* mimetype_from_mime(const char* mime)
{
    for (int i = 0; i < mimetypes_size; i++)
        if (STRINGS_EQUAL(mimetypes[i].mime, mime))
            return &mimetypes[i];

    return &mimetypes[0];
}

const mimetype_t* filename_to_mime(const char* filename)
{
    const char* suffix = strrchr(filename, '.');
    if (suffix == NULL)
        return &mimetypes[0];

    suffix++;
    return mimetype_from_ext(suffix);
}
