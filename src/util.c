#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef DEBUG_PRINT_BACKTRACE
#include <execinfo.h>
#endif 

#include "util.h"


#ifdef DEBUG_PRINT_BACKTRACE
#ifndef DEBUG_BACKTRACE_MAXLENGTH
#define DEBUG_BACKTRACE_MAXLENGTH 20
#endif

void *_backtrace_buffer[DEBUG_BACKTRACE_MAXLENGTH];
#endif 

inline void _debug_print_backtrace() __attribute__((always_inline));
inline void _debug_print_backtrace()
{
#ifdef DEBUG_PRINT_BACKTRACE
    size_t size = backtrace(_backtrace_buffer, DEBUG_BACKTRACE_MAXLENGTH);
    char **symbols = backtrace_symbols(_backtrace_buffer, size);
    if (symbols == NULL)
    {
        fprintf(stderr, "\tBacktrace failed\n");
        return;
    }
    
    fprintf(stderr, "\tBacktrace:\n");
    for(int i = 0; i < size; i++)
    {
        fprintf(stderr, "\t[%02d] %s\n", i, symbols[i]);
    }
    
    free(symbols);    
#endif
}

int _error_print(const char *scope, const char *call, const char *file, int line)
{
    fprintf(stderr, "Error: [%s] %s\n\tIn file %s at line %d\n\t", scope, call, file, line);
    perror(NULL);
    _debug_print_backtrace();
    return -1;
}

int _error_print_exit(const char *scope, const char *call, const char *file, int line)
{
    _error_print(scope, call, file, line);
    exit(EXIT_FAILURE);
}

void _info_print(const char *message)
{
    _info_print_format("%s", message);
}

void _info_print_format(const char *format, ...)
{
    printf("Info: ");
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

static int debug_verbose = 0;
void debug_set_verbose(int verbose)
{
    debug_verbose = verbose;
}

void _debug_print(const char *message)
{
    _debug_print_format("%s", message);
}

void _debug_print_format(const char *format, ...)
{
    if (!debug_verbose)
    {
        return;
    }
    
    printf("Debug: ");
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

unsigned long hash(const char *string)
{
    // djb2 - simple string hashing
    
    unsigned long hash = 5381;
    int c;
    
    while ((c = *string++))
    {
        hash = ((hash << 5) + hash) + c; // hash*33 + c
    }
    
    return hash;
}