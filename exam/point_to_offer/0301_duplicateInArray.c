#include <stdio.h>
#include <stdlib.h>

int duplicate(int arr[], int n) {
	int i, j, tmp;

	for (i = 0; i < n; ++i) {
		while (arr[i] != i) {
			if (arr[i] == arr[arr[i]])
				return arr[i];
			tmp = arr[i];
			arr[i] = arr[tmp];
			arr[tmp] = tmp;
		}
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
