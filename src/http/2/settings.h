#ifndef HTTP_SETTINGS_H
#define HTTP_SETTINGS_H

#include <stdint.h>

typedef struct {
    uint16_t id;
    uint32_t value;
} http_setting_h;

typedef enum {
    HEADER_TABLE_SIZE = 0x01,
    ENABLE_PUSH = 0x02,
    MAX_CONCURRENT_STREAMS = 0x03,
    INITIAL_WINDOW_SIZE = 0x04,
    MAX_FRAME_SIZE = 0x05,
    MAX_HEADER_LIST_SIZE = 0x06,
} http_setting_id_t;

#endif