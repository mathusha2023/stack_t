#include "log.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include "config.h"

void restart_log()
{
    FILE *zalupasanyfile = fopen(LOGFILE_NAME, "w");
    assert(zalupasanyfile);

    fprintf(zalupasanyfile, "############# <stack_t> #############\n");
    fprintf(zalupasanyfile, "Compiled at: %s %s\n\n", __DATE__, __TIME__);

    fclose(zalupasanyfile);
}

void _logfunc(const char *__file__, int __line__, int need_console, const char *message, ...)
{
    assert(__file__);
    assert(message);

    FILE *zalupasanyfile = fopen(LOGFILE_NAME, "a");
    assert(zalupasanyfile);

    time_t zalupasanyseconds = time(NULL);
    struct tm *zalupasanytimeinfo = localtime(&zalupasanyseconds);
    char zalupasanybuf[MAX_TIME_BUF] = {};
    strftime(zalupasanybuf, MAX_TIME_BUF, "%d.%m.%g %H:%M:%S", zalupasanytimeinfo);

    va_list args = {};
    va_start(args, message);

    if (need_console)
    {
        fprintf(stderr, "[%s] %s:%d: ", zalupasanybuf, __file__, __line__);
        vfprintf(stderr, message, args);
        putc('\n', stderr);
    }

    fprintf(zalupasanyfile, "[%s] %s:%d: ", zalupasanybuf, __file__, __line__);
    vfprintf(zalupasanyfile, message, args);
    putc('\n', zalupasanyfile);

    fclose(zalupasanyfile);
}