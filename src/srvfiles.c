#include "srvfiles.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "macros.h"
#include "types/mimetype.h"

dict_t* files = NULL;

int load_file(const char* dir, const char* filename)
{
    FILE* f;
    char name[1024];
    char* buffer;
    const char* mode;
    long len;

    if (files == NULL)
        files = dict_create();

    sprintf(name, "%s/%s", dir, filename);

    const mimetype_t* mime = filename_to_mime(filename);

    if (mime->text)
        mode = "r";
    else
        mode = "rb";

    f = fopen(name, mode);
    if (f == NULL)
    {
        fprintf(stderr, "ERROR: could not open file %s\n", name);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = calloc(len, sizeof(char));
    fread(buffer, len, sizeof(char), f);
    fclose(f);

    dict_append(files, filename, buffer);
    return 0;
}

void load_files_from(const char* path)
{
    struct dirent* dp;
    DIR* dir;
    
    dir = opendir(path);
    if (dir == NULL)
    {
        fprintf(stderr, "ERROR: could not open directory %s\n", path);
        return;
    }

    while ((dp = readdir(dir)) != NULL)
    {
        if (STRINGS_EQUAL(dp->d_name, ".") || STRINGS_EQUAL(dp->d_name, "..")) continue;

        load_file(path, dp->d_name);
    }

    closedir(dir);
}
