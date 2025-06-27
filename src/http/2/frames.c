#include "frames.h"

#include <stdlib.h>
#include <string.h>

http_frame_t* create_frame(http_frame_type_t type, uint8_t flags, uint32_t stream)
{
    http_frame_t* frame = (http_frame_t*) malloc(sizeof(http_frame_t));
    frame->type = type;
    frame->flags = flags;
    frame->stream_id = stream;
    return frame;
}
