#ifndef STATUS_H
#define STATUS_H

typedef struct {
    int code;
    const char* text;

    const char* full;
} http_status_t;

extern const http_status_t status_codes[];
extern const int status_codes_size;

const http_status_t* status_from_code(int code);
const http_status_t* status_from_text(const char* text);

const char* full_status_text(int code);

#endif