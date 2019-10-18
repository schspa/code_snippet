#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "../seqstack/seqstack.h"

/*
 * 利用栈实现后缀表达式（逆波兰表达式）求值:
 * 	1. 读到操作数，压栈；
 * 	2. 读到操作符，出栈所需数量的操作数，将计算后的结果再压栈；
 * 	3. 读到'\n', 出栈栈顶数据，输出结果；
 *
 * 参考：
 * <C程序设计语言>
 * 		P63 Chapter 4.3
 * <数据结构与算法分析-C语言描述>
 * 		P52 Chapter 3.3.3 (堆栈的应用)
 *
 * 扩展(利用二叉树解决相同问题, 代码见tree/app)：
 * <数据结构与算法分析-C语言描述>
 * 		P70 Chapter 4.2.2 (二叉树的应用)
 */

// Return Value: Operator Type
#define END        0
#define NUMBER     1
#define OPERATOR   2

int getop(char s[]) {
	int i;

	while ((s[0] = getchar()) == ' ' || s[0] == '\t')
		;

	/* END */
	if (s[0] == EOF) {
		s[1] = '\0';
		return END;
	}

	/* NUMBER */
	if (isdigit(s[0])) {
		i = 0;
		while (isdigit(s[++i] = getchar()))
			;
		if (s[i] != EOF)
			ungetc(s[i], stdin);
		s[i] = '\0';
		return NUMBER;
	}

	/* non-END and non-number ---> operator */
	s[1] = '\0';
	return OPERATOR;
}

int main(int argc, char *argv[])
{
	char s[8];
	int optype, op1, op2;

	stack_t *stack = stack_init(16);
	if (stack == NULL)
		return -1;

	while ((optype = getop(s)) != END) {
		switch (optype) {
		case NUMBER:
			stack_push(stack, atoi(s));
			break;
		case OPERATOR:
			switch (s[0]) {
			case '+':
				stack_pop(stack, &op2);
				stack_pop(stack, &op1);
				stack_push(stack, op1 + op2);
				break;
			case '-':
				stack_pop(stack, &op2);
				stack_pop(stack, &op1);
				stack_push(stack, op1 - op2);
				break;
			case '*':
				stack_pop(stack, &op2);
				stack_pop(stack, &op1);
				stack_push(stack, op1 * op2);
				break;
			case '/':
				stack_pop(stack, &op2);
				stack_pop(stack, &op1);
				stack_push(stack, op1 / op2);
				break;
			case '\n':
				stack_pop(stack, &op1);
				printf("%d\n", op1);
				break;
			default:
				fprintf(stderr, "Unknown operator: %c\n", s[0]);
				break;
			}
			break;
		}
	}

	stack_destory(stack);
	return 0;
}
