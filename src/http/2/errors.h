#ifndef HTTP_ERRORS_H
#define HTTP_ERRORS_H

#include <stdint.h>

#include "frames.h"

typedef struct {
    uint8_t value;
    const char* name;
} http_error_t;

extern const http_error_t errors[];

http_frame_t* raise_connection_error(uint32_t curstream, const char* error, uint32_t laststream);

#endif