/**
 * @file This file is for Debug, we can use this file like logcat
 */

/**
 * error detect for unix command
 */
#define syserr(variable, function) \
    if (variable) { \
        perror(function); \
        exit(EXIT_FAILURE); \
    }

/**
 * Define own log and debug output.
 *
 * "__VA_ARG__" is varible MACRO substitue. It can make MACRO printf
 * include formats and arguments.
 */
#ifdef DEBUG
#define DEBUG_PRINT(...) printf("DEBUG: " __VA_ARGS__)
#define eprintf(...) \
    do { \
        fprintf(stderr, "ERROR: "); fprintf(stderr, __VA_ARGS__); \
    } while (0)
#else
#define DEBUG_PRINT(...)
#define eprintf(...) \
    do { \
        fprintf(stderr, "ERROR: "); fprintf(stderr, __VA_ARGS__); \
    } while (0)
#endif
