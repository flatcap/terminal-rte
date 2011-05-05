#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "debug.h"

int debug_trace = 0;

/**
 * debug_printf
 */
void debug_printf (struct _IO_FILE *stream, int *control, int trigger, const char *function, const char *format, ...)
{
	int olderr = errno;
	va_list args;

	if (!stream)
		return;
	if (control && ((*control && !trigger) || (!*control && trigger)))
		return;

	fprintf (stream, "%s ", function);
	va_start (args, format);
	vfprintf (stream, format, args);
	va_end (args);
	errno = olderr;
}

