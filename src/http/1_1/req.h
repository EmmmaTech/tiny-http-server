#ifndef HTTP_11_REQ_H
#define HTTP_11_REQ_H

#include "types/http.h"

const http_req_t* extract_req_info(const char input[], int len);

#endif