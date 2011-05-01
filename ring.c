#include "ring.h"

/**
 * ring_init
 */
ring *
ring_init (int rows)
{
	ring *r = NULL;

	if ((rows < 1) || (rows > 1000))
		return NULL;

	r = malloc (sizeof (ring));
	if (!r)
		return NULL;

	r->rows = rows;
	r->head = 0;
	r->tail = 0;
	r->buffer = calloc (rows, sizeof (ring *));

	if (!r->buffer) {
		free (r);
		r = NULL;
	}

	return r;
}

/**
 * ring_free
 */
void
ring_free (ring *r)
{
	int i;

	if (!r)
		return;

	for (i = 0; i < r->rows; i++) {
		free (r->buffer[i]);
	}

	free (r);
}

/**
 * ring_insert
 */
int
ring_insert (ring *r, char *text)
{
#if 0
	free (ring_buffer[index][ring_head[index]]);
	ring_buffer[index][ring_head[index]] = strdup (text);
	ring_head[index] = (ring_head[index]+1) % ring_size;
#endif
	if (!r)
		return 0;

	r->head = (r->head + 1) % r->rows;

	if (r->head == r->tail)		// ring full
		return 0;

	free (r->buffer[r->head]);
	r->buffer[r->head] = text;

	return 1;
}


