#include "display.h"
#include "utils.h"
#include <stdio.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

/* Couleurs ANSI pour le terminal */
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_CYAN   "\x1b[36m"
#define ANSI_RESET  "\x1b[0m"

static void print_with_color(t_entry entry)
{
    if (S_ISDIR(entry.info.st_mode))
        printf(ANSI_BLUE "%s" ANSI_RESET, entry.name);
    else if (S_ISLNK(entry.info.st_mode))
        printf(ANSI_CYAN "%s" ANSI_RESET, entry.name);
    else
        printf("%s", entry.name);
}

static void display_long_format(t_entry entry, t_options *opts)
{
    char perms[11];
    char time_buf[64];

    struct passwd *pw = getpwuid(entry.info.st_uid);
    struct group  *gr = getgrgid(entry.info.st_gid);
    struct tm *tm_info = localtime(&entry.info.st_mtime);

    format_permissions(entry.info.st_mode, perms);

    if (tm_info)
        strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);
    else
        snprintf(time_buf, sizeof(time_buf), "??? ?? ??:??");

    printf("%s %2ld %-8s %-8s ",
           perms,
           (long)entry.info.st_nlink,
           pw ? pw->pw_name : "unknown",
           gr ? gr->gr_name : "unknown");

    if (opts->option_h) {
        char human_size[32];
        format_size_readable(entry.info.st_size, human_size);
        printf("%8s ", human_size);
    } else {
        printf("%8ld ", (long)entry.info.st_size);
    }

    printf("%s ", time_buf);
    print_with_color(entry);
    printf("\n");
}

void display_entries(t_entry *entries, int count, t_options *opts)
{
    for (int i = 0; i < count; i++) {
        if (opts->option_l) {
            display_long_format(entries[i], opts);
        } else {
            print_with_color(entries[i]);

            if (i < count - 1) {
                if (opts->option_m)
                    printf(", ");
                else
                    printf("  ");
            }
        }
    }

    if (!opts->option_l)
        printf("\n");
}
