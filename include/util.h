#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

#define return_defer(code)                                                     \
    do {                                                                       \
        result = code;                                                         \
        goto defer;                                                            \
    } while (0)

#define LOG_ERROR(format, ...)                                                 \
    fprintf(stderr, "ERROR: %s:%d: " format "\n", __FILE__, __LINE__,          \
            ##__VA_ARGS__)

#define LOG_INFO(format, ...)                                                  \
    fprintf(stdout, "INFO: " format "\n", ##__VA_ARGS__)

#endif // UTIL_H
