#include <stdio.h>

const char *last_error = NULL;

const char *gum_error()
{
    return last_error;
}

void gum_log(const char *log)
{
    last_error = log;
    fprintf(stderr, "%s\n", log);
}
