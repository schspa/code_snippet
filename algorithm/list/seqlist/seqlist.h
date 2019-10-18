#ifndef __SEQLIST_H__
#define __SEQLIST_H__

typedef struct _list_t_ list_t;

list_t *list_init(int max);
int list_destory(list_t *list);
int list_insert(list_t *list, int data);
int list_delete(list_t *list, int data);
int list_modify(list_t *list, int old, int new);
int list_search(list_t *list, int data);

typedef int (*datavisit_t)(int data);
int list_foreach(list_t *list, datavisit_t visit);

#endif /* __SEQLIST_H__ */
