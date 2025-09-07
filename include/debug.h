#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define VERBOSE 0
#define DEBUG 1
#define ERROR 1

#if VERBOSE
#define verbose_printf(...) do{ printf(__VA_ARGS__); }while(0)
#else
#define verbose_printf(...)
#endif

#if DEBUG
#define debug_printf(...) do{ printf(__VA_ARGS__); }while(0)
#else
#define debug_printf(...)
#endif

#if ERROR
#define error_printf(...) do{ printf(__VA_ARGS__); }while(0)
#else
#define error_printf(...)
#endif


#endif /* DEBUG_H */
