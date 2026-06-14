
#ifndef ERROR_H
#define ERROR_H


# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h


// Codes d'erreur internes a myls    

typedef enum e_myls_error
{
    ERR_NONE            = 0,  
    ERR_NO_SCH_FILE    = 1,
    ERR_PERMISSION      = 2, 
    ERR_INVALID_OPTION  = 3, 
    ERR_MALLOC          = 4,  
    ERR_OPENDIR         = 5, 
} t_myls_error;


/* Affiche un message d'erreur formate vers stderr.
   Retourne le code d'erreur passe en parametre. */
int     myls_error(t_myls_error code, const char *context);

int     myls_usage(const char *prog_name, char invalid_opt);

/* Retourne le message texte associe a un code d'erreur interne. */
const char *myls_strerror(t_myls_error code);

#endif 