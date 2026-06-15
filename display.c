#define _POSIX_C_SOURCE 200809L
#include "display.h"
#include "utils.h"
#include <ctype.h>
#include <fnmatch.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>

#ifndef S_ISVTX
# define S_ISVTX 01000
#endif

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

static const char	*next_ls_color_token(const char *p, const char **end)
{
	int	escaped;

	escaped = 0;
	while (*p && *p == ':')
		p++;
	*end = p;
	while (**end) {
		if (!escaped && **end == ':')
			break;
		if (!escaped && **end == '\\')
			escaped = 1;
		else
			escaped = 0;
		(*end)++;
	}
	return p;
}

static char	decode_escape_char(const char **src)
{
	int	value;
	int	count;

	(*src)++;
	if (**src == 'e' || **src == 'E') {
		(*src)++;
		return '\033';
	}
	if (**src == 'n') {
		(*src)++;
		return '\n';
	}
	if (**src == 'r') {
		(*src)++;
		return '\r';
	}
	if (**src == 't') {
		(*src)++;
		return '\t';
	}
	if (**src == 'x') {
		(*src)++;
		value = 0;
		count = 0;
		while (isxdigit((unsigned char)**src) && count < 2) {
			value *= 16;
			if (**src >= '0' && **src <= '9')
				value += **src - '0';
			else
				value += tolower((unsigned char)**src) - 'a' + 10;
			(*src)++;
			count++;
		}
		return (char)value;
	}
	if (**src >= '0' && **src <= '7') {
		value = 0;
		count = 0;
		while (**src >= '0' && **src <= '7' && count < 3) {
			value = value * 8 + (**src - '0');
			(*src)++;
			count++;
		}
		return (char)value;
	}
	if (**src)
		return *((*src)++);
	return '\\';
}

static void	copy_ls_value(char *dst, size_t dst_size,
	const char *start, const char *end)
{
	size_t	i;

	i = 0;
	while (start < end && i + 1 < dst_size) {
		if (*start == '\\')
			dst[i++] = decode_escape_char(&start);
		else
			dst[i++] = *start++;
	}
	dst[i] = '\0';
}

static int	ls_colors_lookup(const char *key, char *value, size_t value_size)
{
	const char	*env;
	const char	*token;
	const char	*end;
	const char	*eq;
	size_t		key_len;
	int			found;

	env = getenv("LS_COLORS");
	if (!env || !*env)
		return 0;
	key_len = strlen(key);
	found = 0;
	token = env;
	while (*token) {
		token = next_ls_color_token(token, &end);
		eq = memchr(token, '=', (size_t)(end - token));
		if (eq && (size_t)(eq - token) == key_len
			&& strncmp(token, key, key_len) == 0) {
			copy_ls_value(value, value_size, eq + 1, end);
			found = 1;
		}
		token = (*end == ':') ? end + 1 : end;
	}
	return found;
}

static int	ls_colors_match_extension(const char *name,
	char *value, size_t value_size)
{
	const char	*env;
	const char	*token;
	const char	*end;
	const char	*eq;
	char		pattern[256];
	int			found;

	env = getenv("LS_COLORS");
	if (!env || !*env)
		return 0;
	found = 0;
	token = env;
	while (*token) {
		token = next_ls_color_token(token, &end);
		eq = memchr(token, '=', (size_t)(end - token));
		if (eq && eq > token && *token == '*'
			&& (size_t)(eq - token) < sizeof(pattern)) {
			copy_ls_value(pattern, sizeof(pattern), token, eq);
			if (fnmatch(pattern, name, FNM_PERIOD) == 0) {
				copy_ls_value(value, value_size, eq + 1, end);
				found = 1;
			}
		}
		token = (*end == ':') ? end + 1 : end;
	}
	return found;
}

static int	is_broken_symlink(const t_entry *entry)
{
	struct stat	target;

	return S_ISLNK(entry->lstat_info.st_mode)
		&& stat(entry->path, &target) == -1;
}

static int	is_other_writable(mode_t mode)
{
	return (mode & S_IWOTH) != 0;
}

static int	entry_ls_color_key(const t_entry *entry, char *key, size_t key_size)
{
	mode_t	mode;

	mode = entry->info.st_mode;
	if (is_broken_symlink(entry))
		return snprintf(key, key_size, "or") > 0;
	if (S_ISLNK(entry->lstat_info.st_mode))
		return snprintf(key, key_size, "ln") > 0;
	if (S_ISDIR(mode)) {
		if ((mode & S_ISVTX) && is_other_writable(mode))
			return snprintf(key, key_size, "tw") > 0;
		if (is_other_writable(mode))
			return snprintf(key, key_size, "ow") > 0;
		if (mode & S_ISVTX)
			return snprintf(key, key_size, "st") > 0;
		return snprintf(key, key_size, "di") > 0;
	}
	if (S_ISFIFO(mode))
		return snprintf(key, key_size, "pi") > 0;
	if (S_ISSOCK(mode))
		return snprintf(key, key_size, "so") > 0;
	if (S_ISBLK(mode))
		return snprintf(key, key_size, "bd") > 0;
	if (S_ISCHR(mode))
		return snprintf(key, key_size, "cd") > 0;
	if (mode & S_ISUID)
		return snprintf(key, key_size, "su") > 0;
	if (mode & S_ISGID)
		return snprintf(key, key_size, "sg") > 0;
	if (mode & (S_IXUSR | S_IXGRP | S_IXOTH))
		return snprintf(key, key_size, "ex") > 0;
	return snprintf(key, key_size, "fi") > 0;
}

static const char	*default_entry_color_code(const t_entry *entry)
{
	if (S_ISDIR(entry->info.st_mode))
		return "34";
	if (S_ISLNK(entry->lstat_info.st_mode))
		return "36";
	if (S_ISSOCK(entry->info.st_mode))
		return "35";
	if (S_ISFIFO(entry->info.st_mode))
		return "33";
	if (entry->info.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
		return "32";
	return NULL;
}

static int	entry_color_code(const t_entry *entry, char *code, size_t code_size)
{
	const char	*env;
	const char	*default_code;
	char		key[8];

	env = getenv("LS_COLORS");
	if (!env || !*env) {
		default_code = default_entry_color_code(entry);
		if (!default_code)
			return 0;
		snprintf(code, code_size, "%s", default_code);
		return 1;
	}
	if (entry_ls_color_key(entry, key, sizeof(key))
		&& ls_colors_lookup(key, code, code_size))
		return code[0] != '\0';
	if (S_ISREG(entry->info.st_mode)
		&& ls_colors_match_extension(entry->name, code, code_size))
		return code[0] != '\0';
	if (ls_colors_lookup("no", code, code_size)
		|| ls_colors_lookup("fi", code, code_size))
		return code[0] != '\0';
	return 0;
}

static void	print_color_sequence(const char *code)
{
	char	lc[32];
	char	rc[32];

	if (!ls_colors_lookup("lc", lc, sizeof(lc)))
		snprintf(lc, sizeof(lc), "\033[");
	if (!ls_colors_lookup("rc", rc, sizeof(rc)))
		snprintf(rc, sizeof(rc), "m");
	printf("%s%s%s", lc, code, rc);
}

static void	print_color_reset(void)
{
	char	rs[64];
	char	ec[32];

	if (ls_colors_lookup("ec", ec, sizeof(ec))) {
		printf("%s", ec);
		return;
	}
	if (!ls_colors_lookup("rs", rs, sizeof(rs)))
		snprintf(rs, sizeof(rs), "0");
	print_color_sequence(rs);
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

static void	append_raw(char **dst, char c)
{
	**dst = c;
	(*dst)++;
}

static void	append_text(char **dst, const char *text)
{
	while (*text)
		append_raw(dst, *text++);
}

static void	append_octal_escape(char **dst, unsigned char c)
{
	sprintf(*dst, "\\%03o", c);
	*dst += 4;
}

static void	append_c_escape(char **dst, unsigned char c,
	int escape_double_quote, int escape_space, t_options *opts)
{
	if (opts->option_q && !isprint(c)) {
		append_raw(dst, '?');
		return;
	}
	if (c == '\n')
		append_text(dst, "\\n");
	else if (c == '\t')
		append_text(dst, "\\t");
	else if (c == '\r')
		append_text(dst, "\\r");
	else if (c == '\b')
		append_text(dst, "\\b");
	else if (c == '\f')
		append_text(dst, "\\f");
	else if (c == '\v')
		append_text(dst, "\\v");
	else if (c == '\\')
		append_text(dst, "\\\\");
	else if (c == '"' && escape_double_quote)
		append_text(dst, "\\\"");
	else if (c == ' ' && escape_space)
		append_text(dst, "\\ ");
	else if (isprint(c))
		append_raw(dst, (char)c);
	else
		append_octal_escape(dst, c);
}

static int	shell_needs_quotes(const char *name, int always)
{
	const unsigned char	*p;

	if (always || name[0] == '\0')
		return 1;
	p = (const unsigned char *)name;
	while (*p) {
		if (!(isalnum(*p) || *p == '_' || *p == '@' || *p == '%'
				|| *p == '+' || *p == '=' || *p == ':' || *p == ','
				|| *p == '.' || *p == '/' || *p == '-' || *p == '~'))
			return 1;
		p++;
	}
	return 0;
}

static char	*format_literal_name(const char *name, t_options *opts)
{
	char	*formatted;
	char	*p;

	formatted = malloc(strlen(name) * 4 + 1);
	if (!formatted)
		return NULL;
	p = formatted;
	while (*name) {
		if (opts->option_q && !isprint((unsigned char)*name))
			append_raw(&p, '?');
		else
			append_raw(&p, *name);
		name++;
	}
	*p = '\0';
	return formatted;
}

static char	*format_escape_name(const char *name, t_options *opts)
{
	char	*formatted;
	char	*p;

	formatted = malloc(strlen(name) * 4 + 1);
	if (!formatted)
		return NULL;
	p = formatted;
	while (*name) {
		append_c_escape(&p, (unsigned char)*name, 0, 1, opts);
		name++;
	}
	*p = '\0';
	return formatted;
}

static char	*format_c_name(const char *name, t_options *opts, char quote)
{
	char	*formatted;
	char	*p;

	formatted = malloc(strlen(name) * 4 + 3);
	if (!formatted)
		return NULL;
	p = formatted;
	append_raw(&p, quote);
	while (*name) {
		if (quote == '\'' && *name == '\'')
			append_text(&p, "'\\''");
		else
			append_c_escape(&p, (unsigned char)*name, quote == '"', 0, opts);
		name++;
	}
	append_raw(&p, quote);
	*p = '\0';
	return formatted;
}

static const char	*shell_escape_for_char(unsigned char c)
{
	if (c == '\n')
		return "\\n";
	if (c == '\t')
		return "\\t";
	if (c == '\r')
		return "\\r";
	if (c == '\b')
		return "\\b";
	if (c == '\f')
		return "\\f";
	if (c == '\v')
		return "\\v";
	return NULL;
}

static void	append_shell_escaped_control(char **dst, unsigned char c)
{
	const char	*esc;

	esc = shell_escape_for_char(c);
	append_raw(dst, '\'');
	append_text(dst, "$'");
	if (esc)
		append_text(dst, esc);
	else
		append_octal_escape(dst, c);
	append_raw(dst, '\'');
	append_raw(dst, '\'');
}

static char	*format_shell_name(const char *name, t_options *opts, int always,
	int escape_controls)
{
	char				*formatted;
	char				*p;
	const unsigned char	*s;
	int					quoted;

	quoted = shell_needs_quotes(name, always);
	if (!quoted)
		return format_literal_name(name, opts);
	formatted = malloc(strlen(name) * 12 + 16);
	if (!formatted)
		return NULL;
	p = formatted;
	append_raw(&p, '\'');
	s = (const unsigned char *)name;
	while (*s) {
		if (opts->option_q && !isprint(*s))
			append_raw(&p, '?');
		else if (*s == '\'')
			append_text(&p, "'\\''");
		else if (escape_controls && !isprint(*s))
			append_shell_escaped_control(&p, *s);
		else
			append_raw(&p, (char)*s);
		s++;
	}
	append_raw(&p, '\'');
	*p = '\0';
	return formatted;
}

static char	*format_name(const char *name, t_options *opts)
{
	if (opts->quoting_style == QUOTE_ESCAPE)
		return format_escape_name(name, opts);
	if (opts->quoting_style == QUOTE_C)
		return format_c_name(name, opts, '"');
	if (opts->quoting_style == QUOTE_LOCALE)
		return format_c_name(name, opts, '\'');
	if (opts->quoting_style == QUOTE_SHELL)
		return format_shell_name(name, opts, 0, 0);
	if (opts->quoting_style == QUOTE_SHELL_ALWAYS)
		return format_shell_name(name, opts, 1, 0);
	if (opts->quoting_style == QUOTE_SHELL_ESCAPE)
		return format_shell_name(name, opts, 0, 1);
	if (opts->quoting_style == QUOTE_SHELL_ESCAPE_ALWAYS)
		return format_shell_name(name, opts, 1, 1);
	return format_literal_name(name, opts);
}

static void	print_entry_name(const t_entry *entry, t_options *opts)
{
	char	*name;
	char	indicator;
	char	color[64];
	int		use_color;

	name = format_name(entry->name, opts);
	if (!name)
		return;
	use_color = color_enabled(opts) && entry_color_code(entry, color, sizeof(color));
	if (use_color)
		print_color_sequence(color);
	printf("%s", name);
	if (use_color)
		print_color_reset();
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
	const char	*style;
	const char	*custom;
	const char	*newline;

	t = entry_selected_time(entry, opts);
	tm_info = localtime(&t);
	if (!tm_info) {
		snprintf(buf, len, "??? ?? ??:??");
		return;
	}
	style = opts->option_time_style;
	if (style && strncmp(style, "posix-", 6) == 0)
		style += 6;
	if (style && style[0] == '+') {
		custom = style + 1;
		newline = strchr(custom, '\n');
		if (newline)
			custom = newline + 1;
		if (strftime(buf, len, custom, tm_info) == 0 && len > 0)
			buf[0] = '\0';
		return;
	}
	if (opts->option_full_time
		|| (style && strcmp(style, "full-iso") == 0))
		strftime(buf, len, "%Y-%m-%d %H:%M:%S %z", tm_info);
	else if (style && strcmp(style, "long-iso") == 0)
		strftime(buf, len, "%Y-%m-%d %H:%M", tm_info);
	else if (style && strcmp(style, "iso") == 0)
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
	char	perms[12];
	char	owner_buf[64];
	char	group_buf[64];
	char	size_buf[64];
	char	time_buf[128];
	const char	*owner;
	const char	*group;

	print_prefix_fields(entry, opts, widths);
	format_permissions_entry(entry, perms);
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
