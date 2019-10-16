#include <stdlib.h>
#include "seqstack.h"

stack_t *stack_init(int max) {
	stack_t *stack =  malloc(sizeof(stack_t));
	if (stack == NULL)
		return NULL;

	int *buf = calloc(max, sizeof(int));
	if (buf == NULL) {
		free(stack);
		return NULL;
	}

	stack->buf = buf;
	stack->top = 0;
	stack->max = max;
	return stack;
}

int stack_destory(stack_t *stack) {
	if (stack == NULL)
		return -1;

	free(stack->buf);
	free(stack);
	return 0;
}

int stack_isempty(stack_t *stack) {
	return stack->top == 0;
}

int stack_isfull(stack_t *stack) {
	return stack->top == stack->max;
}

int stack_push(stack_t *stack, int data) {
	if (stack == NULL)
		return -1;

	if (stack_isfull(stack))
		return -1;

	stack->buf[stack->top] = data;
	stack->top++;
	return 0;
}

int stack_pop(stack_t *stack, int *data) {
	if (stack == NULL || data == NULL)
		return -1;

	if (stack_isempty(stack))
		return -1;

	stack->top--;
	*data = stack->buf[stack->top];
	return 0;
}

int stack_foreach(stack_t *stack, datavisit_t visit) {
	if (stack == NULL || visit == NULL)
		return -1;

	int i = 0;
	for (i = stack->top - 1; i >= 0; i--)
		visit(stack->buf[i]);

	return 0;
}
