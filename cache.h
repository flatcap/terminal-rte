#ifndef _CACHE_H_
#define _CACHE_H_

#include "list.h"

typedef struct
{
	LIST *start;
	LIST *end;
	int num_lines;
} CACHE;

CACHE * cache_new (void);
void cache_free (CACHE *c);
void cache_add_line (CACHE *c, char *text);
void cache_dump (CACHE *c);
char * cache_get_line (CACHE *c, int line);
int cache_get_length (CACHE *c);

#endif // _CACHE_H_

