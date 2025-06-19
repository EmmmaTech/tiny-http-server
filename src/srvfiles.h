#ifndef SRVFILES_H
#define SRVFILES_H

#include "containers/dict.h"

extern dict_t* files;

int load_file(const char* dir, const char* filename);
void load_files_from(const char* path);

#endif