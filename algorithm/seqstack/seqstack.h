#ifndef __SEQ_STACK_H__
#define __SEQ_STACK_H__

typedef struct _stack_t_ {
	int *buf;
	int top;
	int max;
} stack_t;


stack_t *stack_init(int max);
int stack_destory(stack_t *stack);
int stack_push(stack_t *stack, int data);
int stack_pop(stack_t *stack, int *data);

typedef int (*datavisit_t)(int data);
int stack_foreach(stack_t *stack, datavisit_t visit);

#endif /* __SEQ_STACK_H__ */
