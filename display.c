#define _POSIX_C_SOURCE 200809L
#include "display.h"
#include "utils.h"
#include <ctype.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>

#define ANSI_BLUE   "\x1b[34m"
#define ANSI_CYAN   "\x1b[36m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_RESET  "\x1b[0m"

typedef struct s_widths {
	int inode;
	int blocks;
	int links;
	int owner;
	int group;
	int author;
	int size;
}	t_widths;

static int	int_width_ll(long long value)
{
	int	width;

	width = 1;
	while (value >= 10) {
		value /= 10;
		width++;
	}
	return width;
}

static int	color_enabled(t_options *opts)
{
	if (opts->color_mode == COLOR_ALWAYS)
		return 1;
	if (opts->color_mode == COLOR_AUTO)
		return isatty(STDOUT_FILENO);
	return 0;
}

static const char	*entry_color(const t_entry *entry)
{
	if (S_ISDIR(entry->info.st_mode))
		return ANSI_BLUE;
	if (S_ISLNK(entry->lstat_info.st_mode))
		return ANSI_CYAN;
	if (S_ISSOCK(entry->info.st_mode))
		return ANSI_MAGENTA;
	if (S_ISFIFO(entry->info.st_mode))
		return ANSI_YELLOW;
	if (entry->info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
		return ANSI_GREEN;
	return NULL;
}

static char	type_indicator(const t_entry *entry, t_options *opts)
{
	if (opts->option_p && S_ISDIR(entry->info.st_mode))
		return '/';
	if (!opts->option_F)
		return '\0';
	if (S_ISDIR(entry->info.st_mode))
		return '/';
	if (S_ISLNK(entry->lstat_info.st_mode))
		return '@';
	if (S_ISFIFO(entry->info.st_mode))
		return '|';
	if (S_ISSOCK(entry->info.st_mode))
		return '=';
	if (!opts->option_file_type
		&& (entry->info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
		return '*';
	return '\0';
}

static void	append_escaped_char(char **dst, unsigned char c, t_options *opts)
{
	if (opts->option_b && !isprint(c)) {
		sprintf(*dst, "\\%03o", c);
		*dst += 4;
	}
	else if (opts->option_q && !isprint(c)) {
		**dst = '?';
		(*dst)++;
	}
	else {
		if (opts->option_Q && (c == '"' || c == '\\')) {
			**dst = '\\';
			(*dst)++;
		}
		**dst = (char)c;
		(*dst)++;
	}
}

static char	*format_name(const char *name, t_options *opts)
{
	char	*formatted;
	char	*p;
	size_t	len;

	len = strlen(name);
	formatted = malloc(len * 4 + 3);
	if (!formatted)
		return NULL;
	p = formatted;
	if (opts->option_Q)
		*p++ = '"';
	while (*name) {
		append_escaped_char(&p, (unsigned char)*name, opts);
		name++;
	}
	if (opts->option_Q)
		*p++ = '"';
	*p = '\0';
	return formatted;
}

static void	print_entry_name(const t_entry *entry, t_options *opts)
{
	char		*name;
	char		indicator;
	const char	*color;

	name = format_name(entry->name, opts);
	if (!name)
		return;
	color = color_enabled(opts) ? entry_color(entry) : NULL;
	if (color)
		printf("%s", color);
	printf("%s", name);
	if (color)
		printf("%s", ANSI_RESET);
	indicator = type_indicator(entry, opts);
	if (indicator)
		putchar(indicator);
	free(name);
}

static int	display_name_width(const t_entry *entry, t_options *opts)
{
	char	*name;
	int		width;

	name = format_name(entry->name, opts);
	if (!name)
		return (int)strlen(entry->name);
	width = (int)strlen(name);
	free(name);
	if (type_indicator(entry, opts))
		width++;
	return width;
}

static const char	*owner_name(const t_entry *entry, t_options *opts,
	char *buf, size_t len)
{
	struct passwd	*pw;

	if (opts->option_n) {
		snprintf(buf, len, "%ld", (long)entry->info.st_uid);
		return buf;
	}
	pw = getpwuid(entry->info.st_uid);
	if (pw)
		return pw->pw_name;
	snprintf(buf, len, "%ld", (long)entry->info.st_uid);
	return buf;
}

static const char	*group_name(const t_entry *entry, t_options *opts,
	char *buf, size_t len)
{
	struct group	*gr;

	if (opts->option_n) {
		snprintf(buf, len, "%ld", (long)entry->info.st_gid);
		return buf;
	}
	gr = getgrgid(entry->info.st_gid);
	if (gr)
		return gr->gr_name;
	snprintf(buf, len, "%ld", (long)entry->info.st_gid);
	return buf;
}

static void	format_time_field(const t_entry *entry, t_options *opts,
	char *buf, size_t len)
{
	struct tm	*tm_info;
	time_t		t;

	t = entry_selected_time(entry, opts);
	tm_info = localtime(&t);
	if (!tm_info) {
		snprintf(buf, len, "??? ?? ??:??");
		return;
	}
	if (opts->option_full_time
		|| (opts->option_time_style
			&& strcmp(opts->option_time_style, "full-iso") == 0))
		strftime(buf, len, "%Y-%m-%d %H:%M:%S %z", tm_info);
	else if (opts->option_time_style
		&& strcmp(opts->option_time_style, "long-iso") == 0)
		strftime(buf, len, "%Y-%m-%d %H:%M", tm_info);
	else if (opts->option_time_style
		&& strcmp(opts->option_time_style, "iso") == 0)
		strftime(buf, len, "%m-%d %H:%M", tm_info);
	else
		strftime(buf, len, "%b %e %H:%M", tm_info);
}

static void	format_size_field(const t_entry *entry, t_options *opts,
	char *buf, size_t len)
{
	if (S_ISCHR(entry->info.st_mode) || S_ISBLK(entry->info.st_mode))
		snprintf(buf, len, "%u, %u", major(entry->info.st_rdev),
			minor(entry->info.st_rdev));
	else
		format_file_size(entry->info.st_size, opts, buf, len);
}

static void	collect_widths(t_entry *entries, int count, t_options *opts,
	t_widths *widths)
{
	char	buf[64];
	char	owner[64];
	char	group[64];
	int		i;
	int		len;

	memset(widths, 0, sizeof(*widths));
	i = 0;
	while (i < count) {
		if (opts->option_i) {
			len = int_width_ll((long long)entries[i].info.st_ino);
			if (len > widths->inode)
				widths->inode = len;
		}
		if (opts->option_s) {
			format_alloc_size(entries[i].info.st_blocks, opts, buf, sizeof(buf));
			len = (int)strlen(buf);
			if (len > widths->blocks)
				widths->blocks = len;
		}
		len = int_width_ll((long long)entries[i].info.st_nlink);
		if (len > widths->links)
			widths->links = len;
		len = (int)strlen(owner_name(&entries[i], opts, owner, sizeof(owner)));
		if (len > widths->owner)
			widths->owner = len;
		len = (int)strlen(group_name(&entries[i], opts, group, sizeof(group)));
		if (len > widths->group)
			widths->group = len;
		if (opts->option_author && len > widths->author)
			widths->author = len;
		format_size_field(&entries[i], opts, buf, sizeof(buf));
		len = (int)strlen(buf);
		if (len > widths->size)
			widths->size = len;
		i++;
	}
}

static void	print_prefix_fields(const t_entry *entry, t_options *opts,
	t_widths *widths)
{
	char	blocks[64];

	if (opts->option_i)
		printf("%*lld ", widths->inode, (long long)entry->info.st_ino);
	if (opts->option_s) {
		format_alloc_size(entry->info.st_blocks, opts, blocks, sizeof(blocks));
		printf("%*s ", widths->blocks, blocks);
	}
}

static void	print_symlink_target(const t_entry *entry)
{
	char	buf[4096];
	ssize_t	len;

	if (!S_ISLNK(entry->lstat_info.st_mode) || entry->stat_followed)
		return;
	len = readlink(entry->path, buf, sizeof(buf) - 1);
	if (len < 0)
		return;
	buf[len] = '\0';
	printf(" -> %s", buf);
}

static void	display_long_entry(const t_entry *entry, t_options *opts,
	t_widths *widths)
{
	char	perms[11];
	char	owner_buf[64];
	char	group_buf[64];
	char	size_buf[64];
	char	time_buf[128];
	const char	*owner;
	const char	*group;

	print_prefix_fields(entry, opts, widths);
	format_permissions(entry->info.st_mode, perms);
	owner = owner_name(entry, opts, owner_buf, sizeof(owner_buf));
	group = group_name(entry, opts, group_buf, sizeof(group_buf));
	format_size_field(entry, opts, size_buf, sizeof(size_buf));
	format_time_field(entry, opts, time_buf, sizeof(time_buf));
	printf("%s %*lld ", perms, widths->links, (long long)entry->info.st_nlink);
	if (!opts->option_g)
		printf("%-*s ", widths->owner, owner);
	if (!opts->option_o && !opts->option_G)
		printf("%-*s ", widths->group, group);
	if (opts->option_author)
		printf("%-*s ", widths->author, owner);
	printf("%*s %s ", widths->size, size_buf, time_buf);
	print_entry_name(entry, opts);
	print_symlink_target(entry);
	putchar(opts->option_zero ? '\0' : '\n');
}

static void	display_total(t_entry *entries, int count, t_options *opts)
{
	blkcnt_t	total;
	char	buf[64];
	int		i;

	total = 0;
	i = 0;
	while (i < count)
		total += entries[i++].info.st_blocks;
	format_alloc_size(total, opts, buf, sizeof(buf));
	printf("total %s\n", buf);
}

static int	terminal_width(t_options *opts)
{
	struct winsize	ws;

	if (opts->option_w && opts->option_w_valeur > 0)
		return opts->option_w_valeur;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
		return ws.ws_col;
	return 80;
}

static void	display_one_per_line(t_entry *entries, int count, t_options *opts)
{
	int	i;

	i = 0;
	while (i < count) {
		print_prefix_fields(&entries[i], opts, &(t_widths){0});
		print_entry_name(&entries[i], opts);
		putchar(opts->option_zero ? '\0' : '\n');
		i++;
	}
}

static void	display_comma(t_entry *entries, int count, t_options *opts)
{
	int	i;

	i = 0;
	while (i < count) {
		print_prefix_fields(&entries[i], opts, &(t_widths){0});
		print_entry_name(&entries[i], opts);
		if (i < count - 1)
			printf(", ");
		i++;
	}
	putchar(opts->option_zero ? '\0' : '\n');
}

static void	display_columns(t_entry *entries, int count, t_options *opts)
{
	int	max_width;
	int	term_width;
	int	cols;
	int	rows;
	int	row;
	int	col;
	int	idx;
	int	width;

	max_width = 0;
	idx = 0;
	while (idx < count) {
		width = display_name_width(&entries[idx], opts);
		if (width > max_width)
			max_width = width;
		idx++;
	}
	max_width += opts->option_T ? opts->option_T_valeur : 2;
	term_width = terminal_width(opts);
	cols = term_width / (max_width > 0 ? max_width : 1);
	if (cols < 1)
		cols = 1;
	rows = (count + cols - 1) / cols;
	row = 0;
	while (row < rows) {
		col = 0;
		while (col < cols) {
			if (opts->option_x)
				idx = row * cols + col;
			else
				idx = col * rows + row;
			if (idx < count) {
				width = display_name_width(&entries[idx], opts);
				print_entry_name(&entries[idx], opts);
				if (col < cols - 1 && idx + rows < count)
					printf("%*s", max_width - width, "");
			}
			col++;
		}
		putchar('\n');
		row++;
	}
}

void display_entries(t_entry *entries, int count, t_options *opts, int show_total)
{
	t_widths	widths;

	if (show_total && opts->option_l)
		display_total(entries, count, opts);
	if (count <= 0)
		return;
	if (opts->option_l) {
		collect_widths(entries, count, opts, &widths);
		for (int i = 0; i < count; i++)
			display_long_entry(&entries[i], opts, &widths);
	}
	else if (opts->option_m)
		display_comma(entries, count, opts);
	else if (opts->option_1 || opts->option_zero || opts->option_i || opts->option_s)
		display_one_per_line(entries, count, opts);
	else
		display_columns(entries, count, opts);
}
