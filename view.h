#ifndef _VIEW_H_
#define _VIEW_H_

#include "cache.h"

#include <gtk/gtk.h>

typedef struct
{
	int cols;
	int rows;
	CACHE *cache;
	GList *windows;

	// Our unpacked copy of the data (no tabs)
	int offset;
	char **data;
} VIEW;

VIEW * view_new           (int cols, int rows);
void   view_free          (VIEW *view);
void   view_add_line      (VIEW *view, char *text);
void   view_dump          (VIEW *view);
char * view_get_line      (VIEW *view, int line);
int    view_get_length    (VIEW *view);
void   view_set_size      (VIEW *view, int cols, int rows);
void   view_add_window    (VIEW *view, GtkWidget *window);
void   view_remove_window (VIEW *view, GtkWidget *window);

#endif // _VIEW_H_

