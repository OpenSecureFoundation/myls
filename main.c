#include <stdio.h>
#include "options.h"

int main(int argc, char **argv) {
	t_options options;
	int path_start;
	int exit_code = 0;
	
	init_options(&options);
	parse_options(argc, argv, &options, &path_start);

	if (path_start == argc) {
		/* TODO : appeler process_directory(".", &options)  */
		printf("Répertoire courant\n"); //temporaire
	} else {
		for (int i = path_start; i < argc; i++) {
			/* TODO : appeler process_directory(argv[i], &options) */
			printf("Chemin : %s\n", argv[i]); //temporaire
		}
	}
	free_options(&options);
	return exit_code;
}
