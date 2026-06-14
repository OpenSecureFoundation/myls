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

static void	parse_time_value(const char *val, t_options *options)
{
	options->option_u = 0;
	options->option_c = 0;
	if (strcmp(val, "atime") == 0 || strcmp(val, "access") == 0
		|| strcmp(val, "use") == 0)
		options->option_u = 1;
	else if (strcmp(val, "ctime") == 0 || strcmp(val, "status") == 0)
		options->option_c = 1;
	else if (strcmp(val, "mtime") != 0 && strcmp(val, "modify") != 0)
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

static void	parse_long_option(const char *arg, t_options *options)
{
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
		options->option_D = 1;
	else if (strcmp(arg, "--classify") == 0) {
		options->option_F = 1;
		options->option_file_type = 0;
	}
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
	else if (strcmp(arg, "--hide-control-chars") == 0)
		options->option_q = 1;
	else if (strcmp(arg, "--quote-name") == 0)
		options->option_Q = 1;
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
		options->option_hyperlink = 1;
	else if (strncmp(arg, "--sort=", 7) == 0)
		parse_sort_value(arg + 7, options);
	else if (strncmp(arg, "--time=", 7) == 0)
		parse_time_value(arg + 7, options);
	else if (strncmp(arg, "--block-size=", 13) == 0)
		options->option_block_size_value = atoi(arg + 13);
	else if (strncmp(arg, "--width=", 8) == 0) {
		options->option_w_valeur = atoi(arg + 8);
		options->option_w = 1;
	}
	else if (strncmp(arg, "--hide=", 7) == 0)
		set_string_option(&options->option_hide_pattern, arg + 7);
	else if (strncmp(arg, "--ignore=", 9) == 0)
		set_string_option(&options->option_ignore_pattern, arg + 9);
	else if (strncmp(arg, "--time-style=", 13) == 0)
		set_string_option(&options->option_time_style, arg + 13);
	else if (strncmp(arg, "--tabsize=", 10) == 0) {
		options->option_T_valeur = atoi(arg + 10);
		options->option_T = 1;
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
		options->option_D = 1;
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
		options->option_Z = 1;
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

void parse_options(int argc, char **argv, t_options *options, int *path_start)
{
	int	i;
	int	j;

	i = 1;
	while (i < argc && argv[i][0] == '-' && argv[i][1] != '\0') {
		if (strcmp(argv[i], "--") == 0) {
			i++;
			break;
		}
		if (argv[i][1] == '-')
			parse_long_option(argv[i], options);
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
				parse_short_option(argv[i][j], options);
				j++;
			}
		}
		i++;
	}
	if (options->option_zero)
		set_display_mode(options, '1');
	*path_start = i;
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
