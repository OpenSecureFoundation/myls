#define _POSIX_C_SOURCE 200809L
#include "directory.h"
#include "error.h"
#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char	*make_child_path(const char *dir, const char *name)
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

static int	is_dot_or_dotdot(const char *name)
{
	return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

static int	is_backup_name(const char *name)
{
	size_t	len;

	len = strlen(name);
	return len > 0 && name[len - 1] == '~';
}

static int	should_skip_name(const char *name, t_options *opts)
{
	if (!opts->option_a) {
		if (opts->option_A && is_dot_or_dotdot(name))
			return 1;
		if (!opts->option_A && name[0] == '.')
			return 1;
	}
	if (opts->option_B && is_backup_name(name))
		return 1;
	if (opts->option_ignore_pattern
		&& fnmatch(opts->option_ignore_pattern, name, 0) == 0)
		return 1;
	if (!opts->option_a && !opts->option_A && opts->option_hide_pattern
		&& fnmatch(opts->option_hide_pattern, name, 0) == 0)
		return 1;
	return 0;
}

static void	free_entries(t_entry *entries, int count)
{
	int	i;

	i = 0;
	while (i < count) {
		free_entry(&entries[i]);
		i++;
	}
	free(entries);
}

t_entry *read_directory(const char *path, t_options *opts, int *count)
{
	DIR				*dir;
	struct dirent	*dp;
	t_entry			*entries;
	t_entry			*tmp;
	char			*fullpath;
	int				capacity;
	int				n;

	*count = 0;
	dir = opendir(path);
	if (!dir) {
		myls_error(ERR_OPENDIR, path);
		return NULL;
	}
	capacity = 64;
	n = 0;
	entries = malloc(capacity * sizeof(t_entry));
	if (!entries) {
		closedir(dir);
		myls_error(ERR_MALLOC, NULL);
		return NULL;
	}
	while ((dp = readdir(dir)) != NULL) {
		if (should_skip_name(dp->d_name, opts))
			continue;
		if (n >= capacity) {
			capacity *= 2;
			tmp = realloc(entries, capacity * sizeof(t_entry));
			if (!tmp) {
				free_entries(entries, n);
				closedir(dir);
				myls_error(ERR_MALLOC, NULL);
				return NULL;
			}
			entries = tmp;
		}
		fullpath = make_child_path(path, dp->d_name);
		if (!fullpath) {
			free_entries(entries, n);
			closedir(dir);
			myls_error(ERR_MALLOC, NULL);
			return NULL;
		}
		if (fill_entry(&entries[n], dp->d_name, fullpath, opts, 0) == 0)
			n++;
		free(fullpath);
	}
	closedir(dir);
	*count = n;
	return entries;
}
