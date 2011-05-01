#ifndef _VIEW_H_
#define _VIEW_H_

#include "cache.h"

typedef struct
{
	int cols;
	int rows;
	CACHE *cache;
} VIEW;

VIEW * view_new (int cols, int rows);
void view_free (VIEW *v);
void view_add_line (VIEW *v, char *text);
void view_dump (VIEW *v);
char * view_get_line (VIEW *v, int line);
int view_get_length (VIEW *v);

#endif // _VIEW_H_

