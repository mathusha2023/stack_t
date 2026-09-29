#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

// #define DISABLE_LOGS
// #define NSTKDEBUG

#define RED_COLOR "\x1b[31m"
#define GREEN_COLOR "\x1b[32m"
#define BLUE_COLOR "\x1b[34m"
#define YELLOW_COLOR "\x1b[33m"
#define CYAN_COLOR "\x1b[36m"
#define GREY_COLOR "\033[90m"
#define RESET_COLOR "\x1b[0m"

#define free_ptr(p) \
    {               \
        free(p);    \
        p = NULL;   \
    }

const char LOGFILE_NAME[] = "log.txt";
const double EPSILON = 1e-6;

#endif // CONFIG_H