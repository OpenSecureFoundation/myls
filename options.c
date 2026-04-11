#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"

void init_options(t_options *options) {
	memset(options, 0, sizeof(t_options));
	options->option_C = 1; //affichage en colonne par défaut
	options->option_T_valeur = 8;
}

static void parse_long_option(const char *arg, t_options *options) {
	if (strcmp(arg, "--all") == 0) options->option_a = 1;
	else if (strcmp(arg, "--almost-all") == 0) options->option_A = 1;
	else if (strcmp(arg, "--author") == 0) options->option_author = 1;
	else if (strcmp(arg, "--escape") == 0) options->option_b = 1;
	else if (strcmp(arg, "--ignore-backups") == 0) options->option_b = 1;
	else if (strcmp(arg, "--directory") == 0) options->option_d = 1;
	else if (strcmp(arg, "--dired") == 0) options->option_D = 1;
	else if (strcmp(arg, "--classify") == 0) options->option_F = 1;
	else if (strcmp(arg, "--file-type") == 0) options->option_F = 1;
	else if (strcmp(arg, "--no-group") == 0) options->option_G = 1;
	else if (strcmp(arg, "--human-readable") == 0) options->option_G = 1;
	else if (strcmp(arg, "--si") == 0) options->option_si = 1;
	else if (strcmp(arg, "--dereference-command-line") == 0) options->option_H = 1;
	else if (strcmp(arg, "--dereference-command-line-symlink-to-dir") == 0) options->option_dereference_command_line_symlink_to_dir = 1;
	else if (strcmp(arg, "--inode") == 0) options->option_i = 1;
	else if (strcmp(arg, "--kibibytes") == 0) options->option_k = 1;
	else if (strcmp(arg, "--dereference") == 0) options->option_L = 1;
	else if (strcmp(arg, "--numeric-uid-gid") == 0) options->option_n = 1;
	else if (strcmp(arg, "--literal") == 0) options->option_N = 1;
	else if (strcmp(arg, "--indicator-style=slash") == 0) options->option_p = 1;
	else if (strcmp(arg, "--hide-control-chars") == 0) options->option_q = 1;
	else if (strcmp(arg, "--quote-name") == 0) options->option_Q = 1;
	else if (strcmp(arg, "--reverse") == 0) options->option_r = 1;
	else if (strcmp(arg, "--recursive") == 0) options->option_R = 1;
	else if (strcmp(arg, "--size") == 0) options->option_s = 1;
	else if (strcmp(arg, "--full-time") == 0) options->option_full_time = 1;
	else if (strcmp(arg, "--color") == 0) options->option_color = 1;
	else if (strncmp(arg, "--color=", 8) == 0) {
		const char *val = arg + 8;
		if (strcmp(val, "always") == 0 || strcmp(val, "auto") == 0)
			options->option_color = 1;
		else
			options->option_color = 0; //--color=never

	} 
	else if (strcmp(arg, "--group-directories-first") == 0) options->option_group_directories_first = 1;
	else if (strcmp(arg, "--zero") == 0) options->option_zero = 1;
	else if (strcmp(arg, "--hyperlink") == 0) options->option_hyperlink = 1;
	else if (strcmp(arg, "--help") == 0) {
		printf("Utilisation: myls [OPTION]... [FILE]...\n");
		printf("Reproduit le comportement de la commande ls.\n");
		exit(0);
	}
	else if (strcmp(arg, "--version") == 0) {
		printf("myls version 1.0 - Equipe 6\n");
		exit(0);
	}
	else if (strncmp(arg, "--sort=", 7) == 0) {
		const char *val = arg + 7;

		if (strcmp(val, "size") == 0) options->option_S = 1;
		else if (strcmp(val, "time") == 0) options->option_t = 1;
		else if (strcmp(val, "none") == 0) options->option_U = 1;
		else if (strcmp(val, "extension") == 0) options->option_X = 1;
		else if (strcmp(val, "verison") == 0) options->option_v = 1;
		else if (strcmp(val, "name") == 0) {/* tri par nom, par défaut */}
		else if (strcmp(val, "width") == 0) {/* tri par largeur */}
	}
	else if (strncmp(arg, "--time=", 7) == 0) {
		const char *val = arg + 7;
		
		if (strcmp(val, "atime") == 0 || strcmp(val, "access") == 0 || strcmp(val, "use") == 0) options->option_u = 1;
		else if (strcmp(val, "ctime") == 0 || strcmp(val, "status") == 0) options->option_c = 1;
		/* mtime est activé par défaut */
	}
	else if (strncmp(arg, "--block-size=", 13) == 0) {
		options->option_block_size_value = atoi(arg + 13);
	}
	else if (strncmp(arg, "--width=", 8) == 0) {
		options->option_w_valeur = atoi(arg + 8);
		options->option_w = 1;
	}
	else if (strncmp(arg, "--hide=", 7) == 0) {
		options->option_hide_pattern = strdup(arg + 7);
	}
	else if (strncmp(arg, "--ignore=", 9) == 0) {
		options->option_ignore_pattern = strdup(arg + 9);
	}
	else if (strncmp(arg, "--time-style=", 13) == 0) {
		options->option_time_style = strdup(arg + 13);
	}
	else if (strncmp(arg, "--tabsize=", 10) == 0) {
		options->option_T_valeur = atoi(arg + 10);
		options->option_T = 1;
	}
	else {
		fprintf(stderr, "myls:option inconnue '%s'\n", arg);
		exit(2);
	}
}

static void parse_short_option(char c, t_options *options) {
	switch (c) {
		case 'a': options->option_a = 1; break;
		case 'A': options->option_A = 1; break;
		case 'b': options->option_b = 1; break;
		case 'B': options->option_B = 1; break;
		case 'c': options->option_c = 1; break;
		case 'C': options->option_C = 1; break;
		case 'd': options->option_d = 1; break;
		case 'D': options->option_D = 1; break;
		case 'f': options->option_a = 1; options->option_U = 1; break;
		case 'F': options->option_F = 1; break;
		case 'g': options->option_l = 1; options->option_g = 1; break;
		case 'G': options->option_G = 1; break;
		case 'h': options->option_h = 1; break;
		case 'H': options->option_H = 1; break;
		case 'i': options->option_i = 1; break;
		case 'k': options->option_k = 1; break;
		case 'l': options->option_l = 1; break;
		case 'L': options->option_L = 1; break;
		case 'm': options->option_m = 1; break;
		case 'n': options->option_l = 1; options->option_n = 1; break;
		case 'N': options->option_N = 1; break;
		case 'o': options->option_l = 1; options->option_o = 1; break;
		case 'p': options->option_p = 1; break;
		case 'q': options->option_q = 1; break;
		case 'Q': options->option_Q = 1; break;
		case 'r': options->option_r = 1; break;
		case 'R': options->option_R = 1; break;
		case 's': options->option_s = 1; break;
		case 'S': options->option_S = 1; break;
		case 't': options->option_t = 1; break;
		case 'u': options->option_u = 1; break;
		case 'U': options->option_U = 1; break;
		case 'v': options->option_v = 1; break;
		case 'x': options->option_x = 1; break;
		case 'X': options->option_X = 1; break;
		case 'Z': options->option_Z = 1; break;
		case '1': options->option_1 = 1; break;
		default:
			fprintf(stderr, "myls: option invalide -- '%c'\n", c);
			fprintf(stderr, "Essayez 'myls --help' pour plus d’information.\n");
			exit(2);
	}
}

void parse_options(int argc, char **argv, t_options *options, int *path_start) {
	int i = 1;
	while (i<argc && argv[i][0]=='-') {
		if (argv[i][1] == '-') { //dans ce cas il s’agit d’une option longue
			if (argv[i][2] == '\0') {
				i++;
				break;
			}
			parse_long_option(argv[i], options);
		}
		else { // option courte
			int j = 1;
			while (argv[i][j]) {
				parse_short_option(argv[i][j], options);
				j++;
			}
		}
		i++;
	}
	*path_start = i;
}

void free_options(t_options *options) {
	if (options->option_hide_pattern) {
		free(options->option_hide_pattern);
		options->option_hide_pattern = NULL;
	}
	if (options->option_ignore_pattern) {
		free(options->option_ignore_pattern);
		options->option_ignore_pattern = NULL;
	}
	if (options->option_time_style) {
		free(options->option_time_style);
		options->option_time_style = NULL;
	}
}
