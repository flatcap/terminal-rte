#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include "view.h"
#include "debug.h"

/**
 * view_new
 */
VIEW *
view_new (int cols, int rows)
{
	VIEW  *view  = NULL;
	CACHE *cache = NULL;

	view = calloc (1, sizeof (VIEW));
	if (!view)
		return NULL;

	cache = cache_new();
	if (!cache) {
		free (view);
		return NULL;
	}

	view->cols  = cols;
	view->rows  = rows;
	view->cache = cache;

	Tprintf ("(%d,%d) -> %p\n", cols, rows, view);
	return view;
}

/**
 * view_free
 */
void
view_free (VIEW *view)
{
	Tprintf ("(%p)\n", view);
	free (view);
}

/**
 * view_add_line
 */
void
view_add_line (VIEW *view, char *text)
{
	if (!view)
		return;

	Tprintf ("(%p,\"%s\")\n", view, text);
	cache_add_line (view->cache, text);
}

/**
 * view_dump
 */
void
view_dump (VIEW *view)
{
	if (!view)
		return;
	printf ("%s\n", __FUNCTION__);
	printf ("\tcols = %d\n", view->cols);
	printf ("\trows = %d\n", view->rows);
	cache_dump (view->cache);
}

/**
 * view_get_line
 */
char *
view_get_line (VIEW *view, int line)
{
	if (!view)
		return NULL;

	Tprintf ("(%p,%d)\n", view, line);
	return cache_get_line (view->cache, line);
}

/**
 * view_get_length
 */
int
view_get_length (VIEW *view)
{
	int len = 0;
	if (!view)
		return 0;

	len = cache_get_length (view->cache);
	Tprintf ("(%p) -> %d\n", view, len);
	return len;
}

/**
 * view_set_size
 */
void
view_set_size (VIEW *view, int cols, int rows)
{
	if (!view)
		return;

	Tprintf ("(%p,%d,%d)\n", view, cols, rows);
	view->cols = cols;
	view->rows = rows;
}

/**
 * view_add_window
 */
void
view_add_window (VIEW *view, GtkWidget *window)
{
	if (!view || !window)
		return;

	view->windows = g_list_append (view->windows, window);
}

/**
 * view_remove_window
 */
void
view_remove_window (VIEW *view, GtkWidget *window)
{
	if (!view || !window)
		return;

	view->windows = g_list_remove (view->windows, window);
}
