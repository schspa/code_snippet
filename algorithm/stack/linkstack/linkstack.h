#ifndef __LINK_STACK_H__
#define __LINK_STACK_H__

/*
 * Link Stack Implement
 */
typedef struct _node_t_ {
	int data;
	struct _node_t_ *next;
} node_t;

typedef struct _stack_t_ {
	node_t *top;
	int len;
	int max;
} stack_t;

node_t *node_alloc(int data);
int node_free(node_t *node);

stack_t *stack_init(int max);
int stack_destory(stack_t *stack);
int stack_isempty(stack_t *stack);
int stack_isfull(stack_t *stack);
int stack_push(stack_t *stack, int data);
int stack_pop(stack_t *stack, int *data);

typedef int (*datavisit_t)(int data);
int stack_foreach(stack_t *stack, datavisit_t visit);

#endif /* __LINK_STACK_H__ */
