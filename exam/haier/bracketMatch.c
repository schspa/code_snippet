#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 判定'('与')'是否匹配，匹配返回括号对的数量，不匹配返回-1。

/**************************************************/
// 利用栈来实现

#define STACK_SIZE 16

typedef struct _stack_t_ {
	int buf[STACK_SIZE];
	int top;
	int max;
} stack_t;

stack_t *stack_init() {
	stack_t *stack =  malloc(sizeof(stack_t));
	if (stack == NULL)
		return NULL;
	stack->top = 0;
	stack->max = STACK_SIZE;
	return stack;
}

int stack_destory(stack_t *stack) {
	if (stack == NULL)
		return -1;
	free(stack);
	return 0;
}

int stack_push(stack_t *stack, int data) {
	if (stack == NULL)
		return -1;
	// stack is full
	if (stack->top == stack->max)
		return -1;
	stack->buf[stack->top] = data;
	stack->top++;
	return 0;
}

int stack_pop(stack_t *stack, int *data) {
	if (stack == NULL || data == NULL)
		return -1;
	// stack is empty
	if (stack->top == 0)
		return -1;
	stack->top--;
	*data = stack->buf[stack->top];
	return 0;
}

int stack_len(stack_t *stack) {
	if (stack == NULL)
		return -1;
	return stack->top;
}

/**************************************************/

int caculate(char s[], int n) {
	stack_t *stack = stack_init();

	int i, count = 0;
	int p, q;
	for (i = 0; i < n; ++i) {
		p = s[i];
		q = 0;
		if (p == '(') {
			stack_push(stack, p);
		} else if (p == ')') {
			stack_pop(stack, &q);
			if (q != '(')
				goto err;
			count++;
		}
	}

	if (stack_len(stack) != 0)
		goto err;

	stack_destory(stack);
	return count;

err:
	stack_destory(stack);
	return -1;
}

int main(int argc, char *argv[])
{
	char s[100] = {0};

	fgets(s, 100, stdin);

	printf("%d\n", caculate(s, strlen(s)));

	return 0;
}
