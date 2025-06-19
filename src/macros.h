#ifndef MACROS_H
#define MACROS_H

#include <string.h>

#define STRINGS_EQUAL(x, y) (strcmp(x, y) == 0)
#define STRINGS_EQUAL_N(x, y, n) (strncmp(x, y, n) == 0)

#define SCUFFED_MEMCPY(dest, src, len) \
                                        for (i = 0; i < len; i++) { dest[i] = *src; src++; }; \
                                        dest[i] = 0;

#endif