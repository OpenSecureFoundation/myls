#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "directory.h"
#include "display.h"
#include "entry.h"
#include "options.h"
#include "sort.h"

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

static int	is_dot_or_dotdot(const char *name)
{
	return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

static int	process_directory(const char *path, t_options *options,
	int print_header, int *printed_section)
{
	t_entry	*entries;
	int		count;
	int		status;
	int		i;

	if (*printed_section)
		printf("\n");
	if (print_header)
		printf("%s:\n", path);
	*printed_section = 1;
	entries = read_directory(path, options, &count);
	if (!entries)
		return 1;
	sort_entries(entries, count, options);
	display_entries(entries, count, options, 1);
	status = 0;
	if (options->option_R) {
		i = 0;
		while (i < count) {
			if (entry_is_dir(&entries[i]) && !is_dot_or_dotdot(entries[i].name)
				&& (options->option_L || !entry_is_symlink(&entries[i]))) {
				if (process_directory(entries[i].path, options, 1,
						printed_section) != 0)
					status = 1;
			}
			i++;
		}
	}
	free_entries(entries, count);
	return status;
}

static int	build_argument_entries(int argc, char **argv, int path_start,
	t_options *options, t_entry **out_entries, int *out_count)
{
	t_entry	*entries;
	int		count;
	int		i;

	count = argc - path_start;
	*out_count = 0;
	*out_entries = NULL;
	if (count <= 0)
		return 0;
	entries = malloc(count * sizeof(t_entry));
	if (!entries)
		return 1;
	i = 0;
	while (i < count) {
		if (fill_entry(&entries[*out_count], argv[path_start + i],
				argv[path_start + i], options, 1) == 0)
			(*out_count)++;
		i++;
	}
	*out_entries = entries;
	return (*out_count == count) ? 0 : 1;
}

static int	display_argument_files(t_entry *entries, int count,
	t_options *options)
{
	int	file_count;
	int	i;

	file_count = 0;
	i = 0;
	while (i < count) {
		if (options->option_d || !entry_is_dir(&entries[i]))
			file_count++;
		i++;
	}
	if (file_count == 0)
		return 0;
	t_entry *files = malloc(file_count * sizeof(t_entry));
	if (!files)
		return 1;
	file_count = 0;
	i = 0;
	while (i < count) {
		if (options->option_d || !entry_is_dir(&entries[i]))
			files[file_count++] = entries[i];
		i++;
	}
	sort_entries(files, file_count, options);
	display_entries(files, file_count, options, 0);
	free(files);
	return 0;
}

static int	process_paths(int argc, char **argv, int path_start,
	t_options *options)
{
	t_entry	*entries;
	int		count;
	int		status;
	int		printed_section;
	int		need_headers;
	int		i;
	int		has_file_output;

	if (path_start == argc) {
		printed_section = 0;
		return process_directory(".", options, options->option_R,
			&printed_section);
	}
	status = build_argument_entries(argc, argv, path_start, options,
			&entries, &count);
	if (count == 0) {
		free(entries);
		return status ? 1 : 0;
	}
	sort_entries(entries, count, options);
	if (display_argument_files(entries, count, options) != 0)
		status = 1;
	has_file_output = 0;
	i = 0;
	while (i < count) {
		if (options->option_d || !entry_is_dir(&entries[i]))
			has_file_output = 1;
		i++;
	}
	need_headers = (argc - path_start > 1);
	printed_section = has_file_output;
	i = 0;
	while (i < count) {
		if (!options->option_d && entry_is_dir(&entries[i])) {
			if (process_directory(entries[i].path, options, need_headers,
					&printed_section) != 0)
				status = 1;
		}
		i++;
	}
	free_entries(entries, count);
	return status ? 1 : 0;
}

int main(int argc, char **argv)
{
	t_options	options;
	int			path_start;
	int			exit_code;

	init_options(&options);
	parse_options(argc, argv, &options, &path_start);
	exit_code = process_paths(argc, argv, path_start, &options);
	free_options(&options);
	return exit_code;
}
