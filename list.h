#ifndef _LIST_H_
#define _LIST_H_

typedef struct _list
{
	struct _list *prev;
	struct _list *next;
	void *data;
} LIST;

LIST * list_new (void);
void list_free (LIST *l);
LIST * list_append (LIST *l, LIST *new);
int list_get_length (LIST *l);
void list_dump (LIST *l);

#endif // _LIST_H_

