#include <stdio.h>
#include <ctype.h>
#include "../seqstack/seqstack.h"

/*
 * 利用栈实现将中缀表达式(普通数学表达式)转换为后缀表达式(逆波兰表达式)
 *
 * <数据结构与算法分析-C语言描述>
 * 		P54-56 Chapter 3.3.3 (堆栈的应用)
 * 		P70    Chapter 4.2.2 (二叉树的应用: 利用二叉树也可以解决该问题，代码见tree/app)
 */

// Return Value: Operator Type
#define END      0
#define ALPHA    1
#define NUMBER   2
#define OPERATOR 3

int getop(char s[]) {
	int i;

	while ((s[0] = getchar()) == ' ' || s[0] == '\t')
		;

	/* END */
	if (s[0] == EOF) {
		s[1] = '\0';
		return END;
	}

	/* alpha */
	if (isalpha(s[0])) {
		i = 0;
		while (isalpha((s[++i] = getchar())))
			;
		if (s[i] != EOF)
			ungetc(s[i], stdin);
		s[i] = '\0';
		return ALPHA;
	}

	/* number */
	if (isdigit(s[0])) {
		i = 0;
		while (isdigit((s[++i] = getchar())))
			;
		if (s[i] != EOF)
			ungetc(s[i], stdin);
		s[i] = '\0';
		return NUMBER;
	}

	/* non-EOF and non-alpha and non-number ---> operator */
	s[1] = '\0';
	return OPERATOR;
}

int main(int argc, char *argv[])
{
	char s[8] = {0};
	int optype, ch;

	stack_t *stack = stack_init(16);
	if (stack == NULL)
		return -1;

	while ((optype = getop(s)) != END) {
		switch (optype) {
		case ALPHA:
		case NUMBER:
			printf("%s", s);
			break;
		case OPERATOR:
			switch (s[0]) {
			case '(':
				stack_push(stack, s[0]);
				break;
			case ')':
				while (stack_pop(stack, &ch) != -1 && ch != '(')
					putchar(ch);
				if (ch != '(')
					goto err;
				break;
			case '+':
			case '-':
				while (stack_pop(stack, &ch) != -1 && ch != '(')
					putchar(ch);
				if (ch == '(')
					stack_push(stack, ch);
				stack_push(stack, s[0]);
				break;
			case '*':
			case '/':
				while (stack_pop(stack, &ch) != -1 && ch != '(' && ch != '+' && ch != '-')
					putchar(ch);
				if (ch == '(' || ch == '+' || ch == '-')
					stack_push(stack, ch);
				stack_push(stack, s[0]);
				break;
			default:
				fprintf(stderr, "Unknown operator: %c\n", s[0]);
				break;
			}
			break;
		}
	}

	while (stack_pop(stack, &ch) != -1 && ch != '(')
		putchar(ch);
	if (ch == '(')
		goto err;

	putchar('\n');

	stack_destory(stack);
	return 0;

err:
	printf("Error!\n");
	stack_destory(stack);
	return -1;
}
