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

int compare_by_mtime(const void *x, const void *y){
    FileEntry *fichier1 = (FileEntry *)x ;
    FileEntry *fichier2 = (FileEntry *)y ;
    if (fichier1 -> info.st_mtime < fichier2 -> info.st_mtime){ //date de modif la plus récente
        return 1; 
    }
    if(fichier1 -> info.st_mtime > fichier2 -> info.st_mtime){
        return -1;
    }
    return 0;
}

int compare_by_size(const void *x, const void *y){
     FileEntry *fichier1 = (FileEntry *)x ;
     FileEntry *fichier2 = (FileEntry *)y ;
     if(fichier1 -> info.st_size < fichier2 -> info.st_size){
        return 1;
     }
     if(fichier1 -> info.st_size > fichier2 -> info.st_size){
        return -1;
     }
     return 0;
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

void reverse_sort(FileEntry *fichiers, int n){
    int start = 0;
    int end = n-1;
    while(start<end){
        FileEntry temp = fichiers[start];
        fichiers[start] = fichiers[end];
        fichiers[end] = temp;
        start++ ;
        end-- ;
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
