#include "error.h"

static const char *g_error_message[] = {
    "success",
    "No such file or directory",
    "Permission denied",
    "invalid option",
    "memory allocation failure",
    "cannot open directory",
    "cannot access file information"
};

#define ERROR_TABLE_SIZE ((int)(sizeof(g_error_message) / sizeof(g_error_message[0])))

const char *myls_strerror(t_myls_error code)
{
    if (code < 0 || code >= ERROR_TABLE_SIZE)
        return "unknown error";

    return g_error_message[code];
}

int myls_error(t_myls_error code, const char *context)
{
    if (code == ERR_NO_SUCH_FILE
        || code == ERR_PERMISSION
        || code == ERR_OPENDIR
        || code == ERR_STAT)
    {
        if (context && context[0] != '\0')
            fprintf(stderr, "myls: cannot access '%s': %s\n",
                    context, strerror(errno));
        else
            fprintf(stderr, "myls: %s\n", strerror(errno));

        return (int)code;
    }

    if (context && context[0] != '\0')
        fprintf(stderr, "myls: %s: %s\n", context, myls_strerror(code));
    else
        fprintf(stderr, "myls: %s\n", myls_strerror(code));

    return (int)code;
}

int myls_usage(const char *prog_name, char invalid_opt)
{
    fprintf(stderr,
            "%s: invalid option -- '%c'\n"
            "Usage: %s [OPTION]... [FILE]...\n"
            "Try '%s --help' for more information.\n",
            prog_name,
            invalid_opt,
            prog_name,
            prog_name);

    return (int)ERR_INVALID_OPTION;
}
