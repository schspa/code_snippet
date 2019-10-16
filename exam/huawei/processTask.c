#include <stdio.h>

#define MAX 128

// m个处理器可以并发处理n个任务，任务k的处理时间为tm[k];
// 求m个处理器处理完n个任务所消耗的总时间。
void sort(int s[], int n) {
	int i = 0, j = 0, tmp = 0;

	for (i = 1; i < n; i++) {
		tmp = s[i];
		for (j = i; j > 0 && tmp < s[j - 1]; j--)
			s[j] = s[j - 1];
		s[j] = tmp;
	}
}

int min(int s[], int n) {
	int i = 0, min_index = 0;

	for (i = 1; i < n; i++) {
		if (s[i] < s[min_index])
			min_index = i;
	}

	return min_index;
}

int max(int s[], int n) {
	int i = 0, max_index = 0;

	for (i = 1; i < n; i++) {
		if (s[i] > s[max_index])
			max_index = i;
	}

	return max_index;
}

int caculate(int m, int n, int s[]) {
	int i = 0;
	int tm[MAX] = {0};

	sort(s, n);

	// m >= n，处理器数量大于等于作业数量
	if (m >= n)
		return s[n - 1];

	// m < n，处理器数量小于作业数量
	for (i = 0; i < m; i++)
		tm[i] = s[i];
	for (i = m; i < n; i++)
		tm[min(tm, m)] += s[i];

	return tm[max(tm, m)];
}

int main() {
	int m = 0, n = 0;
	int s[MAX] = {0};
	int i = 0;

	scanf("%d %d", &m, &n);
	while (getchar() != '\n')
		;

	for (i = 0; i < n; i++)
		scanf("%d", &s[i]);

	printf("%d\n", caculate(m, n, s));
}
