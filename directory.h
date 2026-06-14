#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "entry.h"
#include "options.h"

t_entry	*read_directory(const char *path, t_options *opts, int *count);
char	*make_child_path(const char *dir, const char *name);

#endif
