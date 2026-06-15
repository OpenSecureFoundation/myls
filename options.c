#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "options.h"

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

void init_options(t_options *options)
{
	memset(options, 0, sizeof(t_options));
	options->option_C = 1;
	options->option_T_valeur = 8;
	options->option_w_valeur = 80;
	options->color_mode = COLOR_NEVER;
	options->sort_mode = SORT_NAME;
}

static void	set_sort_mode(t_options *options, t_sort_mode mode)
{
	options->option_t = 0;
	options->option_S = 0;
	options->option_U = 0;
	options->option_X = 0;
	options->option_v = 0;
	options->sort_mode = mode;
	if (mode == SORT_TIME)
		options->option_t = 1;
	else if (mode == SORT_SIZE)
		options->option_S = 1;
	else if (mode == SORT_NONE)
		options->option_U = 1;
	else if (mode == SORT_EXTENSION)
		options->option_X = 1;
	else if (mode == SORT_VERSION)
		options->option_v = 1;
}

static void	set_display_mode(t_options *options, char mode)
{
	options->option_1 = 0;
	options->option_C = 0;
	options->option_m = 0;
	options->option_x = 0;
	if (mode == '1')
		options->option_1 = 1;
	else if (mode == 'C')
		options->option_C = 1;
	else if (mode == 'm')
		options->option_m = 1;
	else if (mode == 'x')
		options->option_x = 1;
}

static void	parse_error(const char *message, const char *arg)
{
	if (arg)
		fprintf(stderr, "myls: %s '%s'\n", message, arg);
	else
		fprintf(stderr, "myls: %s\n", message);
	exit(2);
}

static void	parse_unsupported(const char *arg)
{
	fprintf(stderr, "myls: unsupported option '%s'\n", arg);
	exit(2);
}

static void	set_string_option(char **dst, const char *value)
{
	char	*copy;

	copy = myls_strdup(value);
	if (!copy) {
		myls_error(ERR_MALLOC, NULL);
		exit(1);
	}
	free(*dst);
	*dst = copy;
}

static void	parse_sort_value(const char *val, t_options *options)
{
	if (strcmp(val, "name") == 0)
		set_sort_mode(options, SORT_NAME);
	else if (strcmp(val, "size") == 0)
		set_sort_mode(options, SORT_SIZE);
	else if (strcmp(val, "time") == 0)
		set_sort_mode(options, SORT_TIME);
	else if (strcmp(val, "none") == 0)
		set_sort_mode(options, SORT_NONE);
	else if (strcmp(val, "extension") == 0)
		set_sort_mode(options, SORT_EXTENSION);
	else if (strcmp(val, "version") == 0)
		set_sort_mode(options, SORT_VERSION);
	else if (strcmp(val, "width") == 0)
		set_sort_mode(options, SORT_WIDTH);
	else
		parse_error("invalid argument for --sort", val);
}

static void	parse_format_value(const char *val, t_options *options)
{
	if (strcmp(val, "verbose") == 0 || strcmp(val, "long") == 0)
		options->option_l = 1;
	else if (strcmp(val, "commas") == 0)
		set_display_mode(options, 'm');
	else if (strcmp(val, "horizontal") == 0 || strcmp(val, "across") == 0)
		set_display_mode(options, 'x');
	else if (strcmp(val, "vertical") == 0)
		set_display_mode(options, 'C');
	else if (strcmp(val, "single-column") == 0)
		set_display_mode(options, '1');
	else
		parse_error("invalid argument for --format", val);
}

static void	parse_time_value(const char *val, t_options *options)
{
	options->option_u = 0;
	options->option_c = 0;
	if (strcmp(val, "atime") == 0 || strcmp(val, "access") == 0
		|| strcmp(val, "use") == 0)
		options->option_u = 1;
	else if (strcmp(val, "ctime") == 0 || strcmp(val, "status") == 0)
		options->option_c = 1;
	else if (strcmp(val, "mtime") == 0 || strcmp(val, "modify") == 0
		|| strcmp(val, "modification") == 0)
		return;
	else if (strcmp(val, "birth") == 0 || strcmp(val, "creation") == 0)
		parse_unsupported("--time=birth");
	else
		parse_error("invalid argument for --time", val);
}

static void	parse_color_value(const char *val, t_options *options)
{
	options->option_color = 1;
	if (strcmp(val, "never") == 0 || strcmp(val, "no") == 0
		|| strcmp(val, "none") == 0) {
		options->color_mode = COLOR_NEVER;
		options->option_color = 0;
	}
	else if (strcmp(val, "always") == 0 || strcmp(val, "yes") == 0
		|| strcmp(val, "force") == 0)
		options->color_mode = COLOR_ALWAYS;
	else if (strcmp(val, "auto") == 0 || strcmp(val, "tty") == 0
		|| strcmp(val, "if-tty") == 0)
		options->color_mode = COLOR_AUTO;
	else
		parse_error("invalid argument for --color", val);
}

static void	parse_classify_value(const char *val, t_options *options)
{
	if (strcmp(val, "never") == 0 || strcmp(val, "no") == 0
		|| strcmp(val, "none") == 0) {
		options->option_F = 0;
		options->option_p = 0;
	}
	else if (strcmp(val, "always") == 0 || strcmp(val, "yes") == 0
		|| strcmp(val, "force") == 0 || strcmp(val, "auto") == 0
		|| strcmp(val, "tty") == 0 || strcmp(val, "if-tty") == 0) {
		options->option_F = 1;
		options->option_file_type = 0;
	}
	else
		parse_error("invalid argument for --classify", val);
}

static void	parse_indicator_style(const char *val, t_options *options)
{
	options->option_F = 0;
	options->option_p = 0;
	options->option_file_type = 0;
	if (strcmp(val, "none") == 0)
		return;
	if (strcmp(val, "slash") == 0)
		options->option_p = 1;
	else if (strcmp(val, "file-type") == 0) {
		options->option_F = 1;
		options->option_file_type = 1;
	}
	else if (strcmp(val, "classify") == 0) {
		options->option_F = 1;
		options->option_file_type = 0;
	}
	else
		parse_error("invalid argument for --indicator-style", val);
}

static void	parse_quoting_style(const char *val, t_options *options)
{
	options->option_b = 0;
	options->option_q = 0;
	options->option_N = 0;
	options->option_Q = 0;
	if (strcmp(val, "literal") == 0 || strcmp(val, "locale") == 0)
		options->option_N = 1;
	else if (strcmp(val, "c") == 0) {
		options->option_Q = 1;
		options->option_b = 1;
	}
	else if (strcmp(val, "escape") == 0)
		options->option_b = 1;
	else if (strncmp(val, "shell", 5) == 0)
		options->option_Q = 1;
	else
		parse_error("invalid argument for --quoting-style", val);
}

static int	parse_size_value(const char *val)
{
	char	*end;
	long	size;

	size = strtol(val, &end, 10);
	if (size <= 0)
		parse_error("invalid argument for --block-size", val);
	if (strcmp(end, "K") == 0 || strcmp(end, "KiB") == 0)
		size *= 1024L;
	else if (strcmp(end, "M") == 0 || strcmp(end, "MiB") == 0)
		size *= 1024L * 1024L;
	else if (strcmp(end, "G") == 0 || strcmp(end, "GiB") == 0)
		size *= 1024L * 1024L * 1024L;
	else if (strcmp(end, "KB") == 0)
		size *= 1000L;
	else if (strcmp(end, "MB") == 0)
		size *= 1000L * 1000L;
	else if (strcmp(end, "GB") == 0)
		size *= 1000L * 1000L * 1000L;
	else if (*end != '\0')
		parse_error("invalid argument for --block-size", val);
	if (size > 2147483647L)
		size = 2147483647L;
	return (int)size;
}

static const char	*long_value(int argc, char **argv, int *i,
	const char *arg, const char *prefix)
{
	size_t	len;

	len = strlen(prefix);
	if (strncmp(arg, prefix, len) == 0 && arg[len] == '=')
		return arg + len + 1;
	if (strcmp(arg, prefix) == 0) {
		if (*i + 1 >= argc)
			parse_error("option requires an argument", arg);
		(*i)++;
		return argv[*i];
	}
	return NULL;
}

static void	parse_long_option(int argc, char **argv, int *i, t_options *options)
{
	const char	*arg;
	const char	*value;

	arg = argv[*i];
	if (strcmp(arg, "--all") == 0)
		options->option_a = 1;
	else if (strcmp(arg, "--almost-all") == 0)
		options->option_A = 1;
	else if (strcmp(arg, "--author") == 0)
		options->option_author = 1;
	else if (strcmp(arg, "--escape") == 0)
		options->option_b = 1;
	else if (strcmp(arg, "--ignore-backups") == 0)
		options->option_B = 1;
	else if (strcmp(arg, "--directory") == 0)
		options->option_d = 1;
	else if (strcmp(arg, "--dired") == 0)
		parse_unsupported(arg);
	else if (strcmp(arg, "--classify") == 0) {
		options->option_F = 1;
		options->option_file_type = 0;
	}
	else if (strncmp(arg, "--classify=", 11) == 0)
		parse_classify_value(arg + 11, options);
	else if (strcmp(arg, "--file-type") == 0) {
		options->option_F = 1;
		options->option_file_type = 1;
	}
	else if (strcmp(arg, "--no-group") == 0)
		options->option_G = 1;
	else if (strcmp(arg, "--human-readable") == 0)
		options->option_h = 1;
	else if (strcmp(arg, "--si") == 0)
		options->option_si = 1;
	else if (strcmp(arg, "--dereference-command-line") == 0)
		options->option_H = 1;
	else if (strcmp(arg, "--dereference-command-line-symlink-to-dir") == 0)
		options->option_dereference_command_line_symlink_to_dir = 1;
	else if (strcmp(arg, "--inode") == 0)
		options->option_i = 1;
	else if (strcmp(arg, "--kibibytes") == 0)
		options->option_k = 1;
	else if (strcmp(arg, "--dereference") == 0)
		options->option_L = 1;
	else if (strcmp(arg, "--numeric-uid-gid") == 0) {
		options->option_l = 1;
		options->option_n = 1;
	}
	else if (strcmp(arg, "--literal") == 0)
		options->option_N = 1;
	else if (strcmp(arg, "--indicator-style=slash") == 0)
		options->option_p = 1;
	else if ((value = long_value(argc, argv, i, arg, "--indicator-style")) != NULL)
		parse_indicator_style(value, options);
	else if (strcmp(arg, "--hide-control-chars") == 0)
		options->option_q = 1;
	else if (strcmp(arg, "--show-control-chars") == 0) {
		options->option_b = 0;
		options->option_q = 0;
	}
	else if (strcmp(arg, "--quote-name") == 0)
		options->option_Q = 1;
	else if ((value = long_value(argc, argv, i, arg, "--quoting-style")) != NULL)
		parse_quoting_style(value, options);
	else if (strcmp(arg, "--reverse") == 0)
		options->option_r = 1;
	else if (strcmp(arg, "--recursive") == 0)
		options->option_R = 1;
	else if (strcmp(arg, "--size") == 0)
		options->option_s = 1;
	else if (strcmp(arg, "--full-time") == 0)
		options->option_full_time = 1;
	else if (strcmp(arg, "--color") == 0)
		parse_color_value("always", options);
	else if (strncmp(arg, "--color=", 8) == 0)
		parse_color_value(arg + 8, options);
	else if (strcmp(arg, "--group-directories-first") == 0)
		options->option_group_directories_first = 1;
	else if (strcmp(arg, "--zero") == 0)
		options->option_zero = 1;
	else if (strcmp(arg, "--hyperlink") == 0)
		parse_unsupported(arg);
	else if (strncmp(arg, "--hyperlink=", 12) == 0)
		parse_unsupported("--hyperlink");
	else if (strcmp(arg, "--context") == 0 || strcmp(arg, "--lcontext") == 0
		|| strcmp(arg, "--scontext") == 0)
		parse_unsupported(arg);
	else if ((value = long_value(argc, argv, i, arg, "--sort")) != NULL)
		parse_sort_value(value, options);
	else if ((value = long_value(argc, argv, i, arg, "--format")) != NULL)
		parse_format_value(value, options);
	else if ((value = long_value(argc, argv, i, arg, "--time")) != NULL)
		parse_time_value(value, options);
	else if ((value = long_value(argc, argv, i, arg, "--block-size")) != NULL)
		options->option_block_size_value = parse_size_value(value);
	else if ((value = long_value(argc, argv, i, arg, "--width")) != NULL) {
		options->option_w_valeur = atoi(value);
		options->option_w = 1;
	}
	else if ((value = long_value(argc, argv, i, arg, "--hide")) != NULL)
		set_string_option(&options->option_hide_pattern, value);
	else if ((value = long_value(argc, argv, i, arg, "--ignore")) != NULL)
		set_string_option(&options->option_ignore_pattern, value);
	else if ((value = long_value(argc, argv, i, arg, "--time-style")) != NULL)
		set_string_option(&options->option_time_style, value);
	else if ((value = long_value(argc, argv, i, arg, "--tabsize")) != NULL) {
		options->option_T_valeur = atoi(value);
		options->option_T = 1;
	}
	else if (strcmp(arg, "--help") == 0) {
		printf("Usage: myls [OPTION]... [FILE]...\n");
		printf("Academic ls implementation. Try README.md for support status.\n");
		exit(0);
	}
	else if (strcmp(arg, "--version") == 0) {
		printf("myls 1.0\n");
		exit(0);
	}
	else
		parse_error("unrecognized option", arg);
}

static void	parse_short_option(char c, t_options *options)
{
	if (c == 'a')
		options->option_a = 1;
	else if (c == 'A')
		options->option_A = 1;
	else if (c == 'b')
		options->option_b = 1;
	else if (c == 'B')
		options->option_B = 1;
	else if (c == 'c')
		options->option_c = 1;
	else if (c == 'C')
		set_display_mode(options, 'C');
	else if (c == 'd')
		options->option_d = 1;
	else if (c == 'D')
		parse_unsupported("-D");
	else if (c == 'f') {
		options->option_f = 1;
		options->option_a = 1;
		set_sort_mode(options, SORT_NONE);
	}
	else if (c == 'F')
		options->option_F = 1;
	else if (c == 'g') {
		options->option_l = 1;
		options->option_g = 1;
	}
	else if (c == 'G')
		options->option_G = 1;
	else if (c == 'h')
		options->option_h = 1;
	else if (c == 'H')
		options->option_H = 1;
	else if (c == 'i')
		options->option_i = 1;
	else if (c == 'k')
		options->option_k = 1;
	else if (c == 'l')
		options->option_l = 1;
	else if (c == 'L')
		options->option_L = 1;
	else if (c == 'm')
		set_display_mode(options, 'm');
	else if (c == 'n') {
		options->option_l = 1;
		options->option_n = 1;
	}
	else if (c == 'N')
		options->option_N = 1;
	else if (c == 'o') {
		options->option_l = 1;
		options->option_o = 1;
	}
	else if (c == 'p')
		options->option_p = 1;
	else if (c == 'q')
		options->option_q = 1;
	else if (c == 'Q')
		options->option_Q = 1;
	else if (c == 'r')
		options->option_r = 1;
	else if (c == 'R')
		options->option_R = 1;
	else if (c == 's')
		options->option_s = 1;
	else if (c == 'S')
		set_sort_mode(options, SORT_SIZE);
	else if (c == 't')
		set_sort_mode(options, SORT_TIME);
	else if (c == 'u')
		options->option_u = 1;
	else if (c == 'U')
		set_sort_mode(options, SORT_NONE);
	else if (c == 'v')
		set_sort_mode(options, SORT_VERSION);
	else if (c == 'x')
		set_display_mode(options, 'x');
	else if (c == 'X')
		set_sort_mode(options, SORT_EXTENSION);
	else if (c == 'Z')
		parse_unsupported("-Z");
	else if (c == '1')
		set_display_mode(options, '1');
	else {
		myls_usage("myls", c);
		exit(2);
	}
}

static int	parse_numeric_argument(int argc, char **argv, int *i, int *j, int *dst)
{
	if (argv[*i][*j + 1] != '\0') {
		*dst = atoi(&argv[*i][*j + 1]);
		return 1;
	}
	if (*i + 1 < argc) {
		(*i)++;
		*dst = atoi(argv[*i]);
		return 1;
	}
	return 0;
}

static int	parse_string_argument(int argc, char **argv, int *i, int *j, char **dst)
{
	if (argv[*i][*j + 1] != '\0') {
		set_string_option(dst, &argv[*i][*j + 1]);
		return 1;
	}
	if (*i + 1 < argc) {
		(*i)++;
		set_string_option(dst, argv[*i]);
		return 1;
	}
	return 0;
}

void parse_options(int argc, char **argv, t_options *options, int *path_start)
{
	int	i;
	int	j;
	int	path_write;
	int	stop_options;

	i = 1;
	path_write = 1;
	stop_options = 0;
	while (i < argc) {
		if (stop_options || argv[i][0] != '-' || argv[i][1] == '\0') {
			argv[path_write++] = argv[i++];
			continue;
		}
		if (strcmp(argv[i], "--") == 0) {
			stop_options = 1;
			i++;
			continue;
		}
		if (argv[i][1] == '-')
			parse_long_option(argc, argv, &i, options);
		else {
			j = 1;
			while (argv[i][j]) {
				if (argv[i][j] == 'T' || argv[i][j] == 'w') {
					if (argv[i][j] == 'T')
						options->option_T = 1;
					else
						options->option_w = 1;
					if (!parse_numeric_argument(argc, argv, &i, &j,
							(argv[i][j] == 'T') ? &options->option_T_valeur
							: &options->option_w_valeur)) {
						fprintf(stderr, "myls: option requires an argument -- '%c'\n",
							argv[i][j]);
						exit(2);
					}
					break;
				}
				if (argv[i][j] == 'I') {
					if (!parse_string_argument(argc, argv, &i, &j,
							&options->option_ignore_pattern)) {
						fprintf(stderr, "myls: option requires an argument -- 'I'\n");
						exit(2);
					}
					break;
				}
				parse_short_option(argv[i][j], options);
				j++;
			}
		}
		i++;
	}
	if (options->option_zero)
		set_display_mode(options, '1');
	*path_start = 1;
	argv[path_write] = NULL;
}

void free_options(t_options *options)
{
	free(options->option_hide_pattern);
	free(options->option_ignore_pattern);
	free(options->option_time_style);
	options->option_hide_pattern = NULL;
	options->option_ignore_pattern = NULL;
	options->option_time_style = NULL;
}
