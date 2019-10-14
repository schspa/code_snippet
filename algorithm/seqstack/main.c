#include <stdio.h>
#include "seqstack.h"

int printf_data(int data) {
	printf("%d ", data);
	return 0;
}

int main(int argc, char *argv[])
{
	stack_t *stack = stack_init(10);
	
	int i = 0;
	for (i = 0; i < 10; ++i) {
		stack_push(stack, i);
		printf("push: i=%d, data=%d\n", i, i);
	}

	stack_foreach(stack, printf_data);
	putchar('\n');

	int data;
	for (i = 0; i < 5; ++i) {
		stack_pop(stack, &data);
		printf("pop: i=%d, data=%d\n", i, data);
	}

	stack_foreach(stack, printf_data);
	putchar('\n');


	stack_push(stack, 55);
	stack_push(stack, 56);
	stack_push(stack, 57);

	for (i = 0; i < 5; ++i) {
		stack_pop(stack, &data);
		printf("pop: i=%d, data=%d\n", i, data);
	}

	stack_foreach(stack, printf_data);
	putchar('\n');

	stack_destory(stack);

	return 0;
}
