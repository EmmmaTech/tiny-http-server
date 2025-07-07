#ifndef MACROS_H
#define MACROS_H

#include <string.h>

#define STRINGS_EQUAL(x, y) (strcmp(x, y) == 0)
#define STRINGS_EQUAL_N(x, y, n) (strncmp(x, y, n) == 0)

#define LEN(x) ((sizeof(x)) / ((sizeof(x[0]))))

#define MIN(x, y) ((x < y) ? (x) : (y))
#define MAX(x, y) ((x > y) ? (x) : (y))

#endif