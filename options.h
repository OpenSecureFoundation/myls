#ifndef OPTIONS_H
#define OPTIONS_H

typedef struct s_options {
	int option_a;
	int option_A;
	
	int option_l;
	int option_1;
	int option_C;
	int option_m;
	int option_x;
	int option_g;
	int option_o;
	int option_n;
	int option_G;

	int option_h;
	int option_s;
	int option_k;

	int option_t;
	int option_r;
	int option_S;
	int option_U;
	int option_v;
	int option_X;
	int option_f;

	int option_u;
	int option_c;

	int option_L;
	int option_H;

	int option_F;
	int option_p;
	int option_i;
	int option_d;
	int option_R;
	int option_Z;

	int option_b;
	int option_q;
	int option_N;
	int option_Q;

	int option_color;

	int option_D;
	int option_B;
	int option_T;
	int option_T_valeur;
	int option_w;
	int option_w_valeur;
	int option_author;
	int option_full_time;
	int option_zero;
	int option_hyperlink;
	int option_group_directories_first;
	int option_si;
	int option_dereference_command_line_symlink_to_dir;

} t_options;

void	init_options(t_options *options);
void	parse_options(int argc, char **argv, t_options *options, int *path_start);


#endif
