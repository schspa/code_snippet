#include <stdio.h>

void mergeArray(int arr1[], int n1, int arr2[], int n2) {
	int i, j, k;

	i = n1 - 1;
	j = n2 - 1;
	k = n1 + n2 - 1;
	while (i >= 0 && j >= 0) {
		if (arr1[i] > arr2[j])
			arr1[k--] = arr1[i--];
		else
			arr1[k--] = arr2[j--];
	}

	if (i < 0) {
		while (k >= 0)
			arr1[k--] = arr2[j--];
	}

}

int main(int argc, char *argv[])
{
	int arr1[10] = {0};
	int arr2[5] = {0};

	int i;
	for (i = 0; i < 5; ++i)
		scanf("%d", &arr1[i]);

	for (i = 0; i < 5; ++i)
		scanf("%d", &arr2[i]);

	mergeArray(arr1, 5, arr2, 5);

	for (i = 0; i < 10; ++i)
		printf("%3d", arr1[i]);
	putchar('\n');

	return 0;
}
