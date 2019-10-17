#include <stdio.h>
#include <string.h>
#include "../seqstack/seqstack.h"

/*
 * 利用栈来实现判定'('与')'是否匹配，匹配返回括号对的数量，不匹配返回-1。
 *
 * <数据结构与算法分析 C语言描述> P76
 */

int caculate(char s[], int n) {
	stack_t *stack = stack_init(16);

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

	if (!stack_isempty(stack))
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
