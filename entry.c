#define _GNU_SOURCE
#include "entry.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int fill_entry(t_entry *entry, const char *name, const char *path)
{
    char fullpath[4096];

    entry->name = strdup(name);
    if (!entry->name)
        return -1;
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, name);
    if (lstat(fullpath, &entry->info) == -1)
        return -1;
    return 0;
}