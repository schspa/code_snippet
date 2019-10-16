#include <stdio.h>
#include <stdlib.h>

// 计算矩阵与其转置的乘积

int caculate(int start, int rows, int columns) {
	int *arr = NULL;
	int i, j, k;

	arr = calloc(rows + columns, sizeof(int));
	for (i = 0; i < rows; i++) {
		for (j = 0; j < columns; j++) {
			arr[i * columns + j] = start++;
		}
	}

	int *result = NULL;
	result = calloc(rows * rows, sizeof(int));
	for (i = 0; i < rows; i++) {
		for (j = 0; j < rows; j++) {
			for (k = 0; k < columns; k++) {
				result[i * rows + j] += arr[i * columns + k]  * arr[j * columns + k];
			}
		}
	}


	for (i = 0; i < rows; i++) {
		for (j = 0; j < rows; j++) {
			printf("%d ", result[i * rows + j]);
		}
		putchar('\n');
	}

	return 0;
}

int main(int argc, char *argv[])
{
	caculate(1, 2, 2);

	return 0;
}
