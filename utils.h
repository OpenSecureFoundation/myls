#ifndef UTILS_H
#define UTILS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "entry.h"
#include "options.h"

void		format_permissions(mode_t mode, char *str);
time_t		entry_selected_time(const t_entry *entry, const t_options *options);
long long	block_unit(const t_options *options);
long long	ceil_div_ll(long long value, long long divisor);
void		format_file_size(off_t size, const t_options *options,
				char *str, size_t size_buf);
void		format_alloc_size(blkcnt_t blocks, const t_options *options,
				char *str, size_t size_buf);

#endif
