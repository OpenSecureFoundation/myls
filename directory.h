#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "entry.h"
#include "options.h"

t_entry *read_directory(const char *path, t_options *opts, int *count);

#endif