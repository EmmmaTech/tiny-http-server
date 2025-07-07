#include "status.h"

#include <stdlib.h>
#include <string.h>

#include "../macros.h"

#define CODE(code, text) { code, text, #code " " text }
const http_status_t status_codes[] = {
    CODE(0, "Unknown Status"),

    CODE(100, "Continue"),
    CODE(101, "Switching Protocols"),

    CODE(200, "OK"),
    CODE(201, "Created"),
    CODE(204, "No Content"),

    CODE(301, "Moved Permanently"),
    CODE(302, "Found"),
    CODE(303, "See Other"),
    CODE(304, "Not Modified"),

    CODE(400, "Bad Request"),
    CODE(401, "Unauthorizied"),
    CODE(403, "Forbidden"),
    CODE(404, "Not Found"),
    CODE(405, "Method Not Allowed"),
    CODE(411, "Length Required"),

    CODE(500, "Internal Server Error"),
    CODE(501, "Not Implemented"),
    CODE(502, "Bad Gateway"),
    CODE(503, "Service Unavailable"),
};
const int status_codes_size = sizeof(status_codes) / sizeof(status_codes[0]);
#undef DEFINE_CODE

const http_status_t* status_from_code(int code)
{
    for (int i = 0; i < status_codes_size; i++)
        if (status_codes[i].code == code)
            return &status_codes[i];

    return &status_codes[0];
}

const http_status_t* status_from_text(const char* text)
{
    for (int i = 0; i < status_codes_size; i++)
        if (STRINGS_EQUAL(status_codes[i].text, text))
            return &status_codes[i];

    return &status_codes[0];
}

const char* full_status_text(int code)
{
    const http_status_t* status = status_from_code(code);
    return status->full;
}
