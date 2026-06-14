#include<stdio.h>
#include<stdlib.h>
#include <sys/stat.h>
typedef struct {
    char name[256]; //file name
    struct stat info; //stocke toutes les infos du fichier
} 
FileEntry ; 

int compare_by_name(const void *x, const void *y){
FileEntry *fichier1 = (FileEntry *)x ;
FileEntry *fichier2 = (FileEntry *)y ;
return strcmp(fichier1->name, fichier2->name);
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

int compare_by_atime(const void *x, const void *y){
    FileEntry *fichier1 = (FileEntry *)x ;
    FileEntry *fichier2 = (FileEntry *)y ;
    if(fichier1 -> info.st_atime < fichier2 -> info.st_atime){
        return 1 ;
    }
    if(fichier1 -> info.st_atime > fichier2 -> info.st_atime){
        return -1 ;
    }
    return 0 ;
} 

int compare_by_ctime(const void *x, const void *y){
    FileEntry *fichier1 = (FileEntry *)x ;
    FileEntry *fichier2 = (FileEntry *)y ;
    if(fichier1 -> info.st_ctime < fichier2 -> info.st_ctime){
        return 1;
    }
    if(fichier1 -> info.st_ctime > fichier2 -> info.st_ctime){
        return -1;
    }
    return 0;
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

void sort_files(FileEntry *fichiers, int n, char choice, int reverse){
    if(choice == "t"){
        qsort(fichiers, n, sizeof(FileEntry), compare_by_mtime);
    }
    else if(choice == "S"){
        qsort(fichiers, n, sizeof(FileEntry), compare_by_size);
    } 
    else if(choice == "u"){
        qsort(fichiers, n, sizeof(FileEntry), compare_by_atime);
    }
    else if(choice == "c"){
        qsort(fichiers, n, sizeof(FileEntry), compare_by_ctime);
    }
    else {
        qsort(fichiers, n, sizeof(FileEntry), compare_by_name);
    }
        if(reverse == 1){
            reverse_sort(fichiers, n);
        }
}
