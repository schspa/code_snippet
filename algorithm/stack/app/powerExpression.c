#include <stdio.h>
#include "../seqstack/seqstack.h"

/*
 * <数据结构与算法分析 C语言描述> P56
 * 利用栈计算多级幂表达式（幂表达式为右结合）
 */

#define __DEBUG__

#ifdef __DEBUG__
#define dprintf(fmt, ...) fprintf(strerr, fmt, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

long caculate(int arr[], int n) {
	if (n == 1)
		return arr[0];

	long ret, y;
	int x, i;
	stack_t *stack = stack_init(16);

	for (i = 0; i < n - 1; ++i)
		stack_push(stack, arr[i]);

	y = arr[n - 1];
	while (!stack_isempty(stack)) {
		stack_pop(stack, &x);
		ret = 1;
		for (i = 0; i < y; ++i)
			ret = ret * x;
		y = ret;
	}

	stack_destory(stack);

	return ret;
}

int main(int argc, char *argv[])
{
	int n, arr[16] = {0};
	int i;

	scanf("%d", &n);
	for (i = 0; i < n; i++)
		scanf("%d", &arr[i]);

	printf("%ld\n", caculate(arr, n));

	return 0;
}
