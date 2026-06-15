#ifndef ENTRY_H
#define ENTRY_H

#include <sys/stat.h>
#include "options.h"

typedef struct s_entry {
	char		*name;
	char		*path;
	struct stat	info;
	struct stat	lstat_info;
	int			is_command_arg;
	int			stat_followed;
} t_entry;

int		fill_entry(t_entry *entry, const char *name, const char *path,
			t_options *options, int is_command_arg);
void	free_entry(t_entry *entry);
int		entry_is_dir(const t_entry *entry);
int		entry_is_symlink(const t_entry *entry);
int		entry_is_dir_for_grouping(const t_entry *entry);

#endif
