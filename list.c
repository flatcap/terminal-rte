#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "debug.h"

/**
 * list_new
 */
LIST *
list_new (void)
{
	LIST *l = calloc (1, sizeof (LIST));
	if (!l)
		return NULL;

	Tprintf ("() -> %p\n", l);
	return l;
}

/**
 * list_free
 */
void
list_free (LIST *l)
{
	LIST *next;

	Tprintf ("(%p)\n", l);
	while (l) {
		//printf ("\tfree: %p, %s\n", l, (char *) l->data);
		next = l->next;
		free (l->data);
		free (l);
		l = next;
	}
}

/**
 * list_append
 */
LIST *
list_append (LIST *l, LIST *new)
{
	if (!l)
		return new;
	Tprintf ("(%p,%p)\n", l, new);
	l->next = new;
	new->prev = l;
	return l;
}

/**
 * list_get_length
 */
int
list_get_length (LIST *l)
{
	int len = 0;

	while (l) {
		len++;
		l = l->next;
	}

	Tprintf ("(%p) -> %d\n", l, len);
	return len;
}

/**
 * list_dump
 */
void
list_dump (LIST *l)
{
	printf ("%s (%p)\n", __FUNCTION__, l);
	while (l) {
		printf ("\t\t\t%p, %p, %s\n", l->prev, l->next, (char *) l->data);
		l = l->next;
	}
}

