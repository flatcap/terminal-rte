#ifndef _DEBUG_H_
#define _DEBUG_H_

extern int debug_trace;

void debug_printf (struct _IO_FILE *stream, int *control, int trigger, const char *function, const char *format, ...);

#define Tprintf(f, a...)	debug_printf (stdout, &debug_trace, 1, __FUNCTION__, f, ##a)

#endif // _DEBUG_H_

