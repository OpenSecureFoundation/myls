#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Codes d'erreur internes à myls */
typedef enum e_myls_error
{
    ERR_NONE = 0,
    ERR_NO_SUCH_FILE = 1,
    ERR_PERMISSION = 2,
    ERR_INVALID_OPTION = 3,
    ERR_MALLOC = 4,
    ERR_OPENDIR = 5,
    ERR_STAT = 6
} t_myls_error;

int myls_error(t_myls_error code, const char *context);
int myls_usage(const char *prog_name, char invalid_opt);
const char *myls_strerror(t_myls_error code);

#endif
