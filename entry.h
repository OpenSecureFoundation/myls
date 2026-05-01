#ifndef ENTRY_H
#define ENTRY_H

#include <sys/stat.h>

typedef struct s_entry {
    char        *name;
    struct stat  info;
} t_entry;

int fill_entry(t_entry *entry, const char *name, const char *path);

#endif