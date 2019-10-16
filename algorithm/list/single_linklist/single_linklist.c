#include <stdlib.h>
#include "single_linklist.h"

struct _node_t_ {
	int data;
	struct _node_t_ *next;
};

struct _list_t_ {
	node_t *head;
	int len;
	int max;
};

node_t *node_alloc(int data) {
	node_t *node = NULL;

	node = malloc(sizeof(node_t));
	if (node == NULL)
		return NULL;

	node->data = data;
	node->next = NULL;
	return node;
}

void node_free(node_t *node) {
	if (node != NULL) {
		free(node);
	}
}

list_t *list_init(int max) {
	list_t *list = NULL;

	list = malloc(sizeof(list_t));
	if (list == NULL)
		return NULL;

	list->head = node_alloc(0);
	if (list->head == NULL) {
		free(list);
		return NULL;
	}

	list->len = 0;
	list->max = max;
	return list;
}

int list_destory(list_t *list) {
	if (list == NULL)
		return 0;

	node_t *node = NULL, *tmp = NULL;
	
	node = list->head;
	while (node != NULL) {
		tmp = node;
		node = node->next;
		node_free(tmp);
	}
	free(list);
	return 0;
}

int list_insert(list_t *list, int data) {
	node_t *node = node_alloc(data);

	if (list == NULL || node == NULL)
		return -1;

	if (list->len >= list->max)
		return -1;

	node->next = list->head->next;
	list->head->next = node;
	list->len++;
	return 0;
}

int list_delete(list_t *list, int data) {
	node_t *node = NULL, *tmp = NULL;

	node = list->head;
	while (node->next != NULL && node->next->data != data)
		node = node->next;

	if (node == NULL)
		return -1;
	
	tmp = node->next;
	node->next = node->next->next;
	node_free(tmp);
	list->len--;
	return 0;
}

int list_modify(list_t *list, int old, int new) {
	node_t *node = NULL;

	node = list->head->next;
	while (node != NULL && node->data != old)
		node = node->next;

	if (node == NULL)
		return -1;
	
	node->data = new;
	return 0;
}

node_t *list_search(list_t *list, int data) {
	node_t *node = NULL;

	node = list->head->next;
	while (node != NULL && node->data != data)
		node = node->next;

	return node;
	
}

int list_reverse(list_t *list) {
	if (list == NULL)
		return -1;

	node_t *node = list->head->next;
	node_t *new = NULL;

	list->head->next = NULL;
	while (node != NULL) {
		new = node;
		node = node->next;

		new->next = list->head->next;
		list->head->next = new;
	}
	return 0;
}

int list_foreach(list_t *list, datavisit_t visit) {
	node_t *node = NULL;

	node = list->head->next;
	while (node != NULL) {
		visit(node->data);
		node = node->next;
	}

	return 0;
}
