#include <stdio.h>
#include "options.h"
#include "directory.h"
#include "entry.h"

int main(int argc, char **argv) {
	t_options options;
	int path_start;
	int exit_code = 0;
	int count = 0;
	
	init_options(&options);
	parse_options(argc, argv, &options, &path_start);

	if (path_start == argc) {
		read_directory(".", &count);
	} else if (argc - path_start == 1) {
		read_directory(argv[path_start], &count);
	} else {
		for (int i = path_start; i < argc; i++) {
			printf("%s:\n", argv[i]);
			read_directory(argv[i], &count);
			if (i < argc-1)
				printf("\n");
		}
	}
	free_options(&options);
	return exit_code;
}
