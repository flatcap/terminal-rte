#include <stdio.h>
#include <stdlib.h>

#include "view.h"

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

	printf ("%s (%d,%d) -> %p\n", __FUNCTION__, cols, rows, view);
	return view;
}

/**
 * view_free
 */
void
view_free (VIEW *view)
{
	printf ("%s (%p)\n", __FUNCTION__, view);
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

	printf ("%s (%p,\"%s\")\n", __FUNCTION__, view, text);
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

	printf ("%s (%p,%d)\n", __FUNCTION__, view, line);
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
	printf ("%s (%p) -> %d\n", __FUNCTION__, view, len);
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

	printf ("%s (%p,%d,%d)\n", __FUNCTION__, view, cols, rows);
	view->cols = cols;
	view->rows = rows;
}

