#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"

/**
 * cache_new
 */
CACHE *
cache_new (void)
{
	CACHE *c = calloc (1, sizeof (CACHE));
	if (!c)
		return NULL;

	printf ("%s() -> %p\n", __FUNCTION__, c);
	return c;
}

/**
 * cache_free
 */
void
cache_free (CACHE *c)
{
	printf ("%s (%p)\n", __FUNCTION__, c);
	free (c);
}

/**
 * cache_add_line
 */
void
cache_add_line (CACHE *c, char *text)
{
	LIST *l = NULL;

	if (!c || !text)
		return;

	l = list_new();
	if (!l)
		return;

	printf ("%s (%p,\"%s\")\n", __FUNCTION__, c, text);
	if (!c->start)
		c->start = l;
	l->data = strdup (text);
	list_append (c->end, l);
	c->end = l;
}

/**
 * cache_dump
 */
void
cache_dump (CACHE *c)
{
	printf ("\t%s\n", __FUNCTION__);
	printf ("\t\tnum_lines = %d\n", c->num_lines);
	printf ("\t\tstart = %p\n", c->start);
	printf ("\t\tend = %p\n", c->end);
	printf ("\t\tlength = %d\n", list_get_length (c->start));
	list_dump (c->start);
}

/**
 * cache_get_line
 */
char *
cache_get_line (CACHE *c, int line)
{
	int i;
	LIST *l;

	if (!c || !c->start || !c->end)
		return NULL;

	if (line >= 0) {
		l = c->start;
		for (i = 0; (i < line) && l; i++) {
			l = l->next;
		}

		if (!l)
			return NULL;
	} else {
		l = c->end;
	}

	printf ("%s (%p,%d) -> %s\n", __FUNCTION__, c, line, (char *) l->data);
	return l->data;
}

/**
 * cache_get_length
 */
int
cache_get_length (CACHE *c)
{
	int len = 0;
	if (!c)
		return 0;
	len = list_get_length (c->start);

	printf ("%s (%p) -> %d\n", __FUNCTION__, c, len);
	return len;
}

