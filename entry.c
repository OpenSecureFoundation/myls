#define _POSIX_C_SOURCE 200809L
#include "entry.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char	*myls_strdup(const char *s)
{
	char	*copy;
	size_t	len;

	len = strlen(s) + 1;
	copy = malloc(len);
	if (!copy)
		return NULL;
	memcpy(copy, s, len);
	return copy;
}

static char	*join_path(const char *dir, const char *name)
{
	char	*path;
	size_t	len_dir;
	size_t	len_name;
	int		need_slash;

	len_dir = strlen(dir);
	len_name = strlen(name);
	need_slash = (len_dir > 0 && dir[len_dir - 1] != '/');
	path = malloc(len_dir + len_name + need_slash + 1);
	if (!path)
		return NULL;
	memcpy(path, dir, len_dir);
	if (need_slash)
		path[len_dir++] = '/';
	memcpy(path + len_dir, name, len_name + 1);
	return path;
}

static int	should_follow_link(const t_entry *entry, t_options *options)
{
	if (options->option_L)
		return 1;
	if (entry->is_command_arg && options->option_H)
		return 1;
	if (entry->is_command_arg
		&& options->option_dereference_command_line_symlink_to_dir)
		return 1;
	return 0;
}

int fill_entry(t_entry *entry, const char *name, const char *path,
	t_options *options, int is_command_arg)
{
	memset(entry, 0, sizeof(*entry));
	entry->is_command_arg = is_command_arg;
	entry->name = myls_strdup(name);
	if (!entry->name)
		return myls_error(ERR_MALLOC, NULL);
	if (path)
		entry->path = myls_strdup(path);
	else
		entry->path = join_path(".", name);
	if (!entry->path) {
		free_entry(entry);
		return myls_error(ERR_MALLOC, NULL);
	}
	if (lstat(entry->path, &entry->lstat_info) == -1) {
		myls_error(ERR_STAT, entry->path);
		free_entry(entry);
		return ERR_STAT;
	}
	entry->info = entry->lstat_info;
	if (S_ISLNK(entry->lstat_info.st_mode) && should_follow_link(entry, options)) {
		if (stat(entry->path, &entry->info) == -1) {
			myls_error(ERR_STAT, entry->path);
			free_entry(entry);
			return ERR_STAT;
		}
		entry->stat_followed = 1;
	}
	return 0;
}

void free_entry(t_entry *entry)
{
	if (!entry)
		return;
	free(entry->name);
	free(entry->path);
	entry->name = NULL;
	entry->path = NULL;
}

int entry_is_dir(const t_entry *entry)
{
	return S_ISDIR(entry->info.st_mode);
}

int entry_is_symlink(const t_entry *entry)
{
	return S_ISLNK(entry->lstat_info.st_mode);
}

int entry_is_dir_for_grouping(const t_entry *entry)
{
	struct stat	target;

	if (S_ISDIR(entry->info.st_mode))
		return 1;
	if (S_ISLNK(entry->lstat_info.st_mode)
		&& stat(entry->path, &target) == 0 && S_ISDIR(target.st_mode))
		return 1;
	return 0;
}
