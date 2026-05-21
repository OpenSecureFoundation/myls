#include "sort.h"
#include <string.h>

static void swap_entries(t_entry *a, t_entry *b)
{
    t_entry tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}

static int compare_by_name(t_entry *a, t_entry *b)
{
    return strcmp(a->name, b->name);
}

static int compare_by_mtime(t_entry *a, t_entry *b)
{
    if (a->info.st_mtime < b->info.st_mtime)
        return 1;
    if (a->info.st_mtime > b->info.st_mtime)
        return -1;
    return compare_by_name(a, b);
}

static int compare_by_atime(t_entry *a, t_entry *b)
{
    if (a->info.st_atime < b->info.st_atime)
        return 1;
    if (a->info.st_atime > b->info.st_atime)
        return -1;
    return compare_by_name(a, b);
}

static int compare_by_ctime(t_entry *a, t_entry *b)
{
    if (a->info.st_ctime < b->info.st_ctime)
        return 1;
    if (a->info.st_ctime > b->info.st_ctime)
        return -1;
    return compare_by_name(a, b);
}

static int compare_by_size(t_entry *a, t_entry *b)
{
    if (a->info.st_size < b->info.st_size)
        return 1;
    if (a->info.st_size > b->info.st_size)
        return -1;
    return compare_by_name(a, b);
}

static const char *get_extension(const char *name)
{
    const char *dot;

    dot = strrchr(name, '.');
    if (dot == NULL || dot == name)
        return "";
    return dot + 1;
}

static int compare_by_extension(t_entry *a, t_entry *b)
{
    int cmp;

    cmp = strcmp(get_extension(a->name), get_extension(b->name));
    if (cmp == 0)
        return compare_by_name(a, b);
    return cmp;
}

static int compare_entries(t_entry *a, t_entry *b, t_options *options)
{
    if (options->option_t)
    {
        if (options->option_u)
            return compare_by_atime(a, b);
        if (options->option_c)
            return compare_by_ctime(a, b);
        return compare_by_mtime(a, b);
    }
    if (options->option_S)
        return compare_by_size(a, b);
    if (options->option_X)
        return compare_by_extension(a, b);
    return compare_by_name(a, b);
}

void sort_entries(t_entry *entries, int count, t_options *options)
{
    int i;
    int j;
    int cmp;

    if (entries == NULL || count <= 1 || options == NULL)
        return;

    if (options->option_U || options->option_f)
        return;

    i = 0;
    while (i < count - 1)
    {
        j = 0;
        while (j < count - i - 1)
        {
            cmp = compare_entries(&entries[j], &entries[j + 1], options);

            if ((!options->option_r && cmp > 0)
                || (options->option_r && cmp < 0))
            {
                swap_entries(&entries[j], &entries[j + 1]);
            }
            j++;
        }
        i++;
    }
}
