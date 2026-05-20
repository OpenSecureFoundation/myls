#ifndef UTILS_H
#define UTILS_H

#include <sys/stat.h>

void format_permissions(__mode_t mode, char *str);

void format_size_readable(__off_t size, char *str);

#endif