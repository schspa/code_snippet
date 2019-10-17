#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "stack.h"
#include "getch.h"

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

#define NUMBER 0

int getop(char s[]) {
	int c, i;

	while ((s[0] = c = getch()) == ' ' || c == '\t')
		;
	s[1] = '\0';

	// it's not a number
	if (!isdigit(c))
		return c;

	// it's a number
	i = 0;
	while (isdigit(s[++i] = c = getch()))
		;
	s[i] = '\0';
	if (c != EOF)
		ungetch(c);
	return NUMBER;
}

int main(int argc, char *argv[])
{
	char s[8];
	int type, op2;

	while ((type = getop(s)) != EOF) {
		switch (type) {
		case NUMBER:
			push(atoi(s));
			break;
		case '+':
			push(pop() + pop());
			break;
		case '-':
			op2 = pop();
			push(pop() - op2);
			break;
		case '*':
			push(pop() * pop());
			break;
		case '/':
			op2 = pop();
			push(pop() / op2);
			break;
		case '\n':
			printf("%d\n", pop());
			break;
		}
	}

	return 0;
}
