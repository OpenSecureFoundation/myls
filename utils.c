#include "utils.h"
#include <stdio.h>

void format_permissions(mode_t mode, char *str) {
    // 1. Initialize the string with dashes
    for(int i = 0; i < 10; i++) str[i] = '-';
    str[10] = '\0'; // Null terminator

    // 2. Check File Type (Directory, Link, or Regular)
    if (S_ISDIR(mode))      str[0] = 'd';
    else if (S_ISLNK(mode)) str[0] = 'l';

    // 3. Check Owner Permissions
    if (mode & S_IRUSR) str[1] = 'r';
    if (mode & S_IWUSR) str[2] = 'w';
    if (mode & S_IXUSR) str[3] = 'x';

    // 4. Check Group Permissions
    if (mode & S_IRGRP) str[4] = 'r';
    if (mode & S_IWGRP) str[5] = 'w';
    if (mode & S_IXGRP) str[6] = 'x';

    // 5. Check Others Permissions
    if (mode & S_IROTH) str[7] = 'r';
    if (mode & S_IWOTH) str[8] = 'w';
    if (mode & S_IXOTH) str[9] = 'x';
}

void format_size_readable(off_t size, char *str) {
    const char *units[] = {"B", "K", "M", "G"};
    int i = 0;
    double d_size = (double)size;

    // Divide by 1024 until the size is small enough
    while (d_size >= 1024 && i < 3) {
        d_size /= 1024;
        i++;
    }

    if (i == 0) sprintf(str, "%ldB", size);
    else sprintf(str, "%.1f%s", d_size, units[i]);
}