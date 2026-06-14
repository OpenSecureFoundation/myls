#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/stat.h>

void format_permissions(mode_t mode, char *str);
void format_size_readable(off_t size, char *str);

#endif
