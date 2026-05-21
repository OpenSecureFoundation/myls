#include "display.h"
#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

void display_entries(t_entry *entries, int count, t_options *opts) {
    for (int i = 0; i < count; i++) {
        if (!opts->option_a && entries[i].name[0] == '.') {
            continue;
        }

        if (opts->option_l) {
            char perms[11];
            format_permissions(entries[i].info.st_mode, perms);
            
            struct passwd *pw = getpwuid(entries[i].info.st_uid);
            struct group  *gr = getgrgid(entries[i].info.st_gid);
            
            char size_str[32];
            if (opts->option_h) {
                format_size_readable(entries[i].info.st_size, size_str);
            } else {
                sprintf(size_str, "%ld", entries[i].info.st_size);
            }

            char time_str[64];
            struct tm *tm_info = localtime(&entries[i].info.st_mtime);
            strftime(time_str, sizeof(time_str), "%b %d %H:%M", tm_info);

            printf("%s %ld %s %s %8s %s %s\n",
                   perms,
                   (long)entries[i].info.st_nlink,
                   pw ? pw->pw_name : "???",
                   gr ? gr->gr_name : "???",
                   size_str,
                   time_str,
                   entries[i].name);
        } else {
            printf("%s  ", entries[i].name);
        }
    }
    if (!opts->option_l && count > 0) {
        printf("\n");
    }
}
