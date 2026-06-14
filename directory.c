#define _GNU_SOURCE
#include "directory.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
t_entry *read_directory(const char *path, t_options *opts, int *count)
{
    DIR           *dir;
    struct dirent *dp;
    t_entry       *entries;
    int            capacity = 64;
    int            n = 0;

    dir = opendir(path);
    if (!dir) {
        perror(path);
        return NULL;
    }
    entries = malloc(capacity * sizeof(t_entry));
    if (!entries) {
        closedir(dir);
        return NULL;
    }
    while ((dp = readdir(dir)) != NULL) {
        if (!opts->option_a && dp->d_name[0] == '.')
            continue;
        if (n >= capacity) {
            capacity *= 2;
            entries = realloc(entries, capacity * sizeof(t_entry));
            if (!entries) {
                closedir(dir);
                return NULL;
            }
        }
        if (fill_entry(&entries[n], dp->d_name, path) == 0)
            n++;
    }
    closedir(dir);
    *count = n;
    return entries;
}
