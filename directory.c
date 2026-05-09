#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "directory.h"
#include "entry.h"

t_entry *read_directory(const char *path, int *count)
{
    DIR           *dir;
    struct dirent *dp;
    t_entry       *entries = NULL;
    int            capacity = 64;
    int            n = 0;

    dir = opendir(path);
    if (!dir) {
        perror(path);
        *count = 0;
        return NULL;
    }

    entries = malloc(sizeof(t_entry) * capacity);
    if (!entries) {
        closedir(dir);
        *count = 0;
        return NULL;
    }

    while ((dp = readdir(dir)) != NULL) {
        if (n >= capacity) {
            capacity *= 2;
            entries = realloc(entries, sizeof(t_entry) * capacity);
            if (!entries) {
                closedir(dir);
                *count = 0;
                return NULL;
            }
        }
        fill_entry(&entries[n], dp->d_name, path);
        n++;
    }

    closedir(dir);
    *count = n;
    return entries;
}
