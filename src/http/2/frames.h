#ifndef HTTP_FRAMES_H
#define HTTP_FRAMES_H

#include <stdint.h>

typedef struct {
    uint32_t length;
    uint8_t type;
    uint8_t flags;
    uint32_t stream_id; // contains reserved bit + stream id (since the stream id can only be 31 bits long)

    const void* payload;
} http_frame_t;

typedef enum {
    DATA = 0x00,
    HEADERS = 0x01,
    PRIORITY = 0x02,
    RST_STREAM = 0x03,
    SETTINGS = 0x04,
    PUSH_PROMISE = 0x05,
    PING = 0x06,
    GOAWAY = 0x07,
    WINDOW_UPDATE = 0x08,
    CONTINUATION = 0x09,
} http_frame_type_t;

http_frame_t* create_frame(http_frame_type_t type, uint8_t flags, uint32_t stream);

#endif