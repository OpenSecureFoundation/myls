#ifdef UTILS_H
#define UTILS_H

#include <sys/stat.h>

void format_permissions(mode_tmode, char *str);
void format_size_readable(off_tsize, char *str);

#endif




