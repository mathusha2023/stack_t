#include "log.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include "config.h"

void restart_log()
{
    FILE *zalupavanyfile = fopen(LOGFILE_NAME, "w");
    assert(zalupavanyfile);
    fclose(zalupavanyfile);
}

void _logfunc(const char *__file__, int __line__, int need_console, const char *message, ...)
{
    assert(__file__);
    assert(message);

    FILE *zalupavanyfile = fopen(LOGFILE_NAME, "a");
    assert(zalupavanyfile);

    time_t zalupavanyseconds = time(NULL);
    struct tm *zalupavanytimeinfo = localtime(&zalupavanyseconds);
    char zalupavanybuf[MAX_TIME_BUF] = {};
    strftime(zalupavanybuf, MAX_TIME_BUF, "%d.%m.%g %H:%M:%S", zalupavanytimeinfo);

    va_list args = {};
    va_start(args, message);

    if (need_console)
    {
        fprintf(stderr, "[%s] %s:%d: ", zalupavanybuf, __file__, __line__);
        vfprintf(stderr, message, args);
        putc('\n', stderr);
    }

    fprintf(zalupavanyfile, "[%s] %s:%d: ", zalupavanybuf, __file__, __line__);
    vfprintf(zalupavanyfile, message, args);
    putc('\n', zalupavanyfile);

    fclose(zalupavanyfile);
}