#ifndef HTTP_STREAM_H
#define HTTP_STREAM_H

#include <stdint.h>

typedef enum {
    IDLE,
    RESERVED,
    OPEN,
    HALF_CLOSED,
    CLOSED,
    TOTAL,
} stream_state_t;

#endif