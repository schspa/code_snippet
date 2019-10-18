#include <stdlib.h>
#include "seqlist.h"

struct _list_t_ {
	int *buf;
	int max;
	int top;
};

list_t *list_init(int max) {
	list_t *list = NULL;
	
	list = malloc(sizeof(list_t));
	if (list == NULL)
		return NULL;
	list->buf = calloc(max, sizeof(int));
	if (list->buf == NULL) {
		free(list);
		return NULL;
	}
	list->max = max;
	list->top = 0;
	return list;
}

int list_destory(list_t *list) {
	if (list == NULL)
		return -1;
	free(list->buf);
	free(list);
	return 0;
}

int list_insert(list_t *list, int data) {
	if (list == NULL)
		return -1;

	/* list is full */
	if (list->top == list->max)
		return -1;

	list->buf[list->top++] = data;
	return 0;
}

int list_delete(list_t *list, int data) {
	if (list == NULL)
		return -1;

	int i, j;
	for (i = 0; i < list->top; i++) {
		if (list->buf[i] == data) {
			for (j = i + 1; j < list->top; j++)
				list->buf[j - 1] = list->buf[j];
			list->top--;
			return 0;
		}
	}

	return -1;
}

int list_modify(list_t *list, int old, int new) {
	if (list == NULL)
		return -1;

	for (int i = 0; i < list->top; ++i) {
		if (list->buf[i] == old) {
			list->buf[i] = new;
			return 0;
		}
	}

	return -1;
}

int list_search(list_t *list, int data) {
	if (list == NULL)
		return 0;
	
	for (int i = 0; i < list->top; ++i) {
		if (list->buf[i] == data)
			return 1;
	}
	return 0;
}

int list_foreach(list_t *list, datavisit_t visit) {
	if (list == NULL)
		return -1;
	
	for (int i = 0; i < list->top; ++i)
		visit(list->buf[i]);
	return 0;
}
