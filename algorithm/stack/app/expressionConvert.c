#include <stdio.h>
#include "../seqstack/seqstack.h"

/*
 * <数据结构与算法分析 C语言描述> P54-56
 * 利用栈实现将普通数学表达式转换为逆波兰表达式
 */
int ischar(char s) {
	return (s >= 'a' && s <= 'z') || (s >= 'A' && s <= 'Z');
}

int isop(char s) {
	return (s == '+' || s == '*' || s == '(' || s == ')');
}

int oppriority(char s1, char s2) {
	switch (s1) {
		case '*':
			if (s2 == '*')
				return 0;
			else
				return 1;
			break;
		case '+':
			if (s2 == '*')
				return -1;
			else
				return 0;
			break;
	}
	
	return 0;
}

void transfer(char s[]) {
	stack_t *stack = stack_init(16);

	int c;
	while (*s != '\0' && *s != '\n') {
		if (*s == ')') {
			while (stack_pop(stack, &c) != -1 && c != '(')
				putchar(c);
			if (c != '(')
				goto err;
		} else if (*s == '(') {
			stack_push(stack, *s);
		} else if (isop(*s)) {
			while (stack_pop(stack, &c) != -1 && c != '(' && oppriority(c, *s) >= 0)
				putchar(c);
			if (oppriority(c, *s) < 0 || c == '(')
				stack_push(stack, c);
			stack_push(stack, *s);
		} else if (ischar(*s)) {
			putchar(*s);
		}

		s++;
	}

	while(stack_pop(stack, &c) != -1) {
		if (c != '(')
			putchar(c);
		else
			goto err;
	}

	putchar('\n');

	stack_destory(stack);
	return;

err:
	printf("\nError!\n");
	stack_destory(stack);
}

int main(int argc, char *argv[])
{
	char s[32] = {0};

	fgets(s, 16, stdin);

	transfer(s);

	return 0;
}
