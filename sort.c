#define _POSIX_C_SOURCE 200809L
#include "sort.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static t_options	*g_sort_options;

static struct timespec	entry_selected_timespec(const t_entry *entry,
	t_options *options)
{
	if (options->option_u)
		return entry->info.st_atim;
	if (options->option_c)
		return entry->info.st_ctim;
	return entry->info.st_mtim;
}

static const char *get_extension(const char *name)
{
	const char *dot;

	dot = strrchr(name, '.');
	if (dot == NULL || dot == name)
		return "";
	return dot + 1;
}

static int	compare_name(const char *a, const char *b)
{
	int	cmp;

	cmp = strcoll(a, b);
	if (cmp == 0)
		cmp = strcmp(a, b);
	return cmp;
}

static int	read_number(const char **s)
{
	long	value;

	value = 0;
	while (**s == '0')
		(*s)++;
	while (isdigit((unsigned char)**s)) {
		value = value * 10 + (**s - '0');
		(*s)++;
		if (value > 2147483647L)
			return 2147483647;
	}
	return (int)value;
}

static int	compare_version_name(const char *a, const char *b)
{
	int	na;
	int	nb;

	while (*a && *b) {
		if (isdigit((unsigned char)*a) && isdigit((unsigned char)*b)) {
			na = read_number(&a);
			nb = read_number(&b);
			if (na != nb)
				return na - nb;
		}
		else {
			if (*a != *b)
				return (unsigned char)*a - (unsigned char)*b;
			a++;
			b++;
		}
	}
	return (unsigned char)*a - (unsigned char)*b;
}

static int	compare_entries_base(const t_entry *a, const t_entry *b,
	t_options *options)
{
	time_t	at;
	time_t	bt;
	struct timespec	ats;
	struct timespec	bts;
	int		cmp;

	if (options->option_group_directories_first
		&& entry_is_dir(a) != entry_is_dir(b))
		return entry_is_dir(a) ? -1 : 1;
	if (options->sort_mode == SORT_TIME) {
		ats = entry_selected_timespec(a, options);
		bts = entry_selected_timespec(b, options);
		at = ats.tv_sec;
		bt = bts.tv_sec;
		if (at < bt)
			return 1;
		if (at > bt)
			return -1;
		if (ats.tv_nsec < bts.tv_nsec)
			return 1;
		if (ats.tv_nsec > bts.tv_nsec)
			return -1;
	}
	else if (options->sort_mode == SORT_SIZE) {
		if (a->info.st_size < b->info.st_size)
			return 1;
		if (a->info.st_size > b->info.st_size)
			return -1;
	}
	else if (options->sort_mode == SORT_EXTENSION) {
		cmp = compare_name(get_extension(a->name), get_extension(b->name));
		if (cmp != 0)
			return cmp;
	}
	else if (options->sort_mode == SORT_VERSION) {
		cmp = compare_version_name(a->name, b->name);
		if (cmp != 0)
			return cmp;
	}
	else if (options->sort_mode == SORT_WIDTH) {
		if (strlen(a->name) != strlen(b->name))
			return (int)(strlen(a->name) - strlen(b->name));
	}
	return compare_name(a->name, b->name);
}

static int	qsort_compare(const void *left, const void *right)
{
	const t_entry	*a;
	const t_entry	*b;
	int				cmp;

	a = (const t_entry *)left;
	b = (const t_entry *)right;
	cmp = compare_entries_base(a, b, g_sort_options);
	if (g_sort_options->option_r)
		cmp = -cmp;
	return cmp;
}

void sort_entries(t_entry *entries, int count, t_options *options)
{
	if (entries == NULL || count <= 1 || options == NULL)
		return;
	if (options->sort_mode == SORT_NONE || options->option_U || options->option_f)
		return;
	g_sort_options = options;
	qsort(entries, count, sizeof(t_entry), qsort_compare);
	g_sort_options = NULL;
}
