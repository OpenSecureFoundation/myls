#include "display.h"
#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

// Colors for the Terminal
#define ANSI_BLUE   
#define ANSI_CYAN   
#define ANSI_RESET  

void print_with_color(t_entry entry) {
    if (S_ISDIR(entry.info.st_mode)) 
        printf(ANSI_BLUE "%s" ANSI_RESET, entry.name);
    else if (S_ISLNK(entry.info.st_mode)) 
        printf(ANSI_CYAN "%s" ANSI_RESET, entry.name);
    else 
        printf("%s", entry.name);
}

void display_long_format(t_entry entry, t_options *opts) {
    char perms[11];
    char time_buf[20];
    
    struct passwd *pw = getpwuid(entry.info.st_uid);
    struct group  *gr = getgrgid(entry.info.st_gid);
    
    format_permissions(entry.info.st_mode, perms);
    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", localtime(&entry.info.st_mtime));

    printf("%s %2ld %-8s %-8s ", 
           perms, 
           entry.info.st_nlink, 
           pw ? pw->pw_name : "unknown", 
           gr ? gr->gr_name : "unknown");

    // Print: Size (Check for -h option)
    if (opts->option_h) {
        char human_size[10];
        format_size_readable(entry.info.st_size, human_size);
        printf("%6s ", human_size);
    } else {
        printf("%8ld ", entry.info.st_size);
    }

    // Print: Date and Colored Name
    printf("%s ", time_buf);
    print_with_color(entry);
    printf("\n");
}

void display_entries(t_entry *entries, int count, t_options *opts) {
    for (int i = 0; i < count; i++) {
        if (opts->option_l) {
            display_long_format(entries[i], opts);
        } else {
            print_with_color(entries[i]);
            
            // Handle -m (comma separated) or standard spacing
            if (i < count - 1) {
                printf(opts->option_m ? ", " : "  ");
            }
        }
    }
    if (!opts->option_l) printf("\n");
}