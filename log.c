#include "log.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h>
#include "config.h"

/*
ТЕПЕРЬ БУДУ ЗНАТЬ ЧТО КАЖДОЕ ИСПОЛЬЗОВАНИЕ va_args ТРЕБУЕТ ОТДЕЛЬНОЙ ИНИЦИАЛИЗАЦИИ
ПРИЧЕМ НА clang++ она даже не нужна, все работает и без нее, но на
g++ указатель уже будет смещен на мусорную память -> сегфолтики.
ИЗ ЭТОГО РЕШЕНИЕ - ПРИ КАЖДОМ ИСПОЛЬЗОВАНИИ ФУНКЦИЙ ВРОДЕ vfprinf
ДЕЛАТЬ va_start и после использования делать va_end

ВЫРАЖАЮ ИСКРЕННЮЮ БЛАГОДАРНОСТЬ ВИТАЛИЯ СИМОНОВУ ЗА НАЙДЕННЫЙ БАГ И ПОМОЩЬ В ЕГО ЛОВЛЕ,
Виталя, без тебя я бы даже не узнал о его существовании
*/

void restart_log()
{
    FILE *zalupasanyfile = fopen(LOGFILE_NAME, "w");
    assert(zalupasanyfile);
    setbuf(zalupasanyfile, NULL);

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
    setbuf(zalupasanyfile, NULL);

    time_t zalupasanyseconds = time(NULL);
    struct tm *zalupasanytimeinfo = localtime(&zalupasanyseconds);
    char zalupasanybuf[MAX_TIME_BUF] = {};
    strftime(zalupasanybuf, MAX_TIME_BUF, "%d.%m.%g %H:%M:%S", zalupasanytimeinfo);

    va_list args = {};

    if (need_console)
    {
        va_start(args, message);
        fprintf(stderr, "[%s] %s:%d: ", zalupasanybuf, __file__, __line__);
        vfprintf(stderr, message, args);
        putc('\n', stderr);
        va_end(args);
    }

    va_start(args, message);
    fprintf(zalupasanyfile, "[%s] %s:%d: ", zalupasanybuf, __file__, __line__);
    vfprintf(zalupasanyfile, message, args);
    putc('\n', zalupasanyfile);
    va_end(args);

    fclose(zalupasanyfile);
}