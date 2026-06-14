#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "directory.h"
#include "entry.h"
#include "display.h"
#include "sort.h"

void process_path(const char *path, t_options *options) {
    int count = 0;
    t_entry *entries = read_directory(path, options, &count);
    if (entries) {
	sort_entries(entries, count, options);
        display_entries(entries, count, options);
        for (int i = 0; i < count; i++) {
            free_entry(&entries[i]);
        }
        free(entries);
    }
}

int main(int argc, char **argv) {
	t_options options;
	int path_start;
	int exit_code = 0;
	
	init_options(&options);
	parse_options(argc, argv, &options, &path_start);

	if (path_start == argc) {
		process_path(".", &options);
	} else {
		for (int i = path_start; i < argc; i++) {
			if (argc - path_start > 1)
				printf("Chemin : %s\n", argv[i]);
			process_path(argv[i], &options);
			if (i < argc-1)
				printf("\n");
		}
	}
	free_options(&options);
	return exit_code;
}
