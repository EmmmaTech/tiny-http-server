#include "errors.h"

#include <stdlib.h>
#include <string.h>

#include "macros.h"

#define ERROR(val, name) { val, name }
const http_error_t errors[] = {
    ERROR(0x00, "NO_ERROR"),
    ERROR(0x01, "PROTOCOL_ERROR"),
    ERROR(0x02, "INTERNAL_ERROR"),
    ERROR(0x03, "FLOW_CONTROL_ERROR"),
    ERROR(0x04, "SETTINGS_TIMEOUT"),
    ERROR(0x05, "STREAM_CLOSED"),
    ERROR(0x06, "FRAME_SIZE_ERROR"),
    ERROR(0x07, "REFUSED_STREAM"),
    ERROR(0x08, "CANCEL"),
    ERROR(0x09, "COMPRESSION_ERROR"),
    ERROR(0x0a, "CONNECT_ERROR"),
    ERROR(0x0b, "ENHANCE_YOUR_CALM"),
    ERROR(0x0c, "INADEQUATE_SECURITY"),
    ERROR(0x0d, "HTTP_1_1_REQUIRED"),
};
const int errors_length = sizeof(errors) / sizeof(errors[0]);
#undef ERROR

struct goaway_data_t {
    uint32_t last_stream_id;
    uint32_t error;
};

http_frame_t* raise_connection_error(uint32_t curstream, const char* error, uint32_t laststream)
{
    http_frame_t* frame = create_frame(GOAWAY, 0x00, curstream);

    struct goaway_data_t* data = (struct goaway_data_t*) malloc(sizeof(struct goaway_data_t));
    data->last_stream_id = laststream;

    uint32_t error_code;
    for (int i = 0; i < errors_length; i++)
        if (STRINGS_EQUAL(errors[i].name, error))
            error_code = errors[i].value;
    else
        error_code = 0x01; // PROTOCOL_ERROR, for when the raised error is unknown

    data->error = error_code;

    frame->length = sizeof(struct goaway_data_t);
    frame->payload = (const struct goaway_data_t*) (data);

    return frame;
}
