#ifndef UTIL_H
#define	UTIL_H

/* error_print* will alway return -1, can be used in the return statement in
 * error handling blocks
 */
#define error_print(call) _error_print(__func__, (call), __FILE__, __LINE__)
#define error_print_scope(scope, call) _error_print((scope), (call), __FILE__, __LINE__)
int _error_print(const char *scope, const char *call, const char *file, int line);

#define error_print_exit(call) _error_print_exit(__func__, (call), __FILE__, __LINE__)
#define error_print_scope_exit(scope, call) _error_print_exit((scope), (call), __FILE__, __LINE__)
int _error_print_exit(const char *scope, const char *call, const char *file, int line);

// Allows adding file/line to the output if needed
#define info_print(msg) _info_print((msg))
#define info_print_format(format, ...) _info_print_format((format), __VA_ARGS__)
void _info_print(const char *message);
void _info_print_format(const char *format, ...);

void debug_set_verbose(int verbose);

// Only prints when verbose mode is active
#define debug_print(msg) _debug_print((msg))
#define debug_print_format(format, ...) _debug_print_format((format), __VA_ARGS__)
void _debug_print(const char *message);
void _debug_print_format(const char *format, ...);

unsigned long hash(const char *string);

#endif	/* UTIL_H */