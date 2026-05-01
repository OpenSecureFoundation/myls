#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "entry.h"

t_entry *read_directory(const char *path, int *count);

#endif