#include <stdio.h>
#include <stdlib.h>

#include "view.h"

/**
 * view_new
 */
VIEW *
view_new (int cols, int rows)
{
	VIEW *v = NULL;
	CACHE *c= NULL;

	v = calloc (1, sizeof (VIEW));
	if (!v)
		return NULL;

	c = cache_new();
	if (!c) {
		free (v);
		return NULL;
	}

	v->cols = cols;
	v->rows = rows;
	v->cache = c;

	printf ("%s (%d,%d) -> %p\n", __FUNCTION__, cols, rows, v);
	return v;
}

/**
 * view_free
 */
void
view_free (VIEW *v)
{
	printf ("%s (%p)\n", __FUNCTION__, v);
	free (v);
}

/**
 * view_add_line
 */
void
view_add_line (VIEW *v, char *text)
{
	if (!v)
		return;

	printf ("%s (%p,\"%s\")\n", __FUNCTION__, v, text);
	cache_add_line (v->cache, text);
}

/**
 * view_dump
 */
void
view_dump (VIEW *v)
{
	if (!v)
		return;
	printf ("%s\n", __FUNCTION__);
	printf ("\tcols = %d\n", v->cols);
	printf ("\trows = %d\n", v->rows);
	cache_dump (v->cache);
}

/**
 * view_get_line
 */
char *
view_get_line (VIEW *v, int line)
{
	if (!v)
		return NULL;

	printf ("%s (%p,%d)\n", __FUNCTION__, v, line);
	return cache_get_line (v->cache, line);
}

/**
 * view_get_length
 */
int
view_get_length (VIEW *v)
{
	int len = 0;
	if (!v)
		return 0;

	len = cache_get_length (v->cache);
	printf ("%s (%p) -> %d\n", __FUNCTION__, v, len);
	return len;
}
