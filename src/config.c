#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"

config_t* config = NULL;

#define PROCESS_BOOL(var) ((strcmp(var, "True") == 0) ? 1 : 0)

void process_variable(char* varname, char* var, int varlen)
{
    if (strcmp(varname, "addr") == 0)
    {
        inet_aton(var, &config->addr);
    }
    if (strcmp(varname, "port") == 0)
    {
        config->port = (in_port_t) atoi(var);
    }
    if (strcmp(varname, "http_version") == 0)
    {
        memcpy(config->version_to_use, var, LEN(config->version_to_use));
    }
    if (strcmp(varname, "tls") == 0)
    {
        config->tls = PROCESS_BOOL(var);
    }
    if (strcmp(varname, "http_fallback") == 0)
    {
        config->http_fallback = PROCESS_BOOL(var);
    }
    if (strcmp(varname, "buffer_size") == 0)
    {
        config->buffer_size = (uint64_t) atoll(var);
    }
    if (strcmp(varname, "connection_limit") == 0)
    {
        config->connection_limit = (uint32_t) atoi(var);
    }
    if (strcmp(varname, "cert") == 0)
    {
        memcpy(config->cert_filename, var, MIN(LEN(config->cert_filename), varlen));
    }
    if (strcmp(varname, "privkey") == 0)
    {
        memcpy(config->privkey_filename, var, MIN(LEN(config->privkey_filename), varlen));
    }

    // any unidentified variable names are ignored
}

int parse_config(const char* filename)
{
    FILE* f;
    char* file;
    long len;

    f = fopen(filename, "r");
    if (!f)
    {
        perror("fopen");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);

    file = calloc(len, sizeof(char));
    fread(file, sizeof(char), len, f);
    fclose(f);

    config = (config_t*) malloc(sizeof(config_t));

    const char *start, *end;
    char *curr, *line_end, varname[100], var[200];
    int curlen;

    start = (const char*) file;
    end = (const char*) (start + len);
    curr = start;

    while (curr != end)
    {
        line_end = curr + strcspn(curr, ";\n");
        memset(varname, 0, LEN(varname));
        memset(var, 0, LEN(var));

        curlen = strcspn(curr, " =");
        if (curr + curlen >= line_end)
        {
            // just skip to the next line
            curr = line_end;
            curr += strcspn(curr, "\n");
            curr += strspn(curr, "\n");
            continue;
        }
        memcpy(varname, curr, MIN(LEN(varname), curlen));

        curr += curlen;
        curr += strspn(curr, " =");

        curlen = strcspn(curr, " ;\n");
        int varlen = MIN(LEN(var), curlen);
        memcpy(var, curr, varlen);

        curr += curlen;
        curr += strcspn(curr, "\n"); // get past any comments that might exist
        curr += strspn(curr, "\n"); // then go to the next line

        process_variable(varname, var, varlen);
    }
}
