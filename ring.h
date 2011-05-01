RING_ROWS

ring_buffer[RING_ROWS]
ring_head

ring_init (int rows)
ring_insert
ring_free (ring *ring)


typedef struct _ring {
	int rows;
	int head;
	int tail;
	void **buffer;
} ring;

