#ifndef __SINGLE_LINK_LIST_H__
#define __SINGLE_LINK_LIST_H__

/*
 * Single Linklist Implement
 */

typedef struct _node_t_ {
	int data;
	struct _node_t_ *next;
} node_t;

typedef struct {
	node_t *head;
	int len;
	int max;
} list_t;

typedef int (*datavisit_t)(int data);

node_t *node_alloc(int data);
void node_free(node_t *node);

list_t *list_init(int max);
int list_destory(list_t *list);
int list_insert(list_t *list, int data);
int list_delete(list_t *list, int data);
int list_modify(list_t *list, int old, int new);
node_t *list_search(list_t *list, int data);
int list_reverse(list_t *list);

int list_foreach(list_t *list, datavisit_t visit);

#endif /* __SINGLE_LINK_LIST_H__ */
