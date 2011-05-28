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

	view->offset = 0;
	view->data   = calloc (rows, sizeof (char *));

	Tprintf ("(%d,%d) -> %p\n", cols, rows, view);
	return view;
}

/**
 * view_free
 */
void
view_free (VIEW *view)
{
	int i = 0;

	if (!view)
		return;

	Tprintf ("(%p)\n", view);

	if (view->data) {
		for (i = 0; i < view->rows; i++) {
			free (view->data[i]);
		}
		free (view->data);
	}

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
	char *data = NULL;

	if (!view || !view->data)
		return NULL;
	return cache_get_line (view->cache, line); // XXX for now
	if ((line < 0) || (line >= view->rows))
		return NULL;

	Tprintf ("(%p,%d)\n", view, line);

	if (!view->data[line]) {		// We don't have a copy of that line
		data = cache_get_line (view->cache, line);
		// detab data
		view->data[line] = data;
	}

	return view->data[line];
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
	int i = 0;

	if (!view || !view->data)
		return;

	Tprintf ("(%p,%d,%d)\n", view, cols, rows);

	if (rows > view->rows) {
		// bigger
		view->data = realloc (view->data, rows * sizeof (char *));
		for (i = view->rows; i < rows; i++) {
			view->data[i] = NULL;
		}
	} else if (rows < view->rows) {
		// smaller
		for (i = rows; i < view->rows; i++) {
			free (view->data[i]);
		}
		view->data = realloc (view->data, rows * sizeof (char *));
	}

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
