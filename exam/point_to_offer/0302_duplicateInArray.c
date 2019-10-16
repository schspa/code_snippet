#include <stdio.h>
#include <stdlib.h>

int countRange(int arr[], int n, int start, int end) {
	int i, count;
	count = 0;
	for (i = 0; i < n; ++i) {
		if (arr[i] >= start && arr[i] <= end)
			count++;
	}
	return count;
}

int duplicate(int arr[], int n) {
	int mid, start, end, count;

	start = 1;
	end = n - 1;
	while (end >= start) {
		mid = ((end - start) >> 1) + start;
		
		count = countRange(arr, n, start, mid);
		if (start == end) {
			if (count > 1)
				return start;
			else
				break;
		}

		if (count > mid - start + 1)
			end = mid;
		else
			start = mid + 1;
	}
	return -1;
}

int main(int argc, char *argv[])
{
	int n, *arr;
	int i;

	scanf("%d", &n);
	arr = calloc(n, sizeof(int));
	if (arr == NULL)
		return -1;

	for (i = 0; i < n; ++i)
		scanf("%d", &arr[i]);

	printf("%d\n", duplicate(arr, n));

	free(arr);

	return 0;
}
