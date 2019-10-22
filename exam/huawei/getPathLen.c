#include <stdio.h>

int get_len(int arr[], int rows, int columns, int start_x, int start_y) {
	int max, len, i, j;

	max = 0;

	// 行
	len = 0;
	for (i = start_x; i < rows; i++) {
		if (arr[i * columns + start_y] == 1)
			len++;
		else
			break;
	}
	if (len > max)
		max = len;

	// 列
	len = 0;
	for (i = start_y; i < columns; i++) {
		if (arr[start_x * columns + i] == 1)
			len++;
		else
			break;
	}
	if (len > max)
		max = len;


	// 正45
	len = 0;
	for (i = start_x, j = start_y; i < rows && j < columns; i++, j++) {
		if (arr[i * columns + j] == 1)
			len++;
		else
			break;
	}
	if (len > max)
		max = len;

	// 负45
	len = 0;
	for (i = start_x, j = start_y; i < rows && j >= 0; i++, j--) {
		if (arr[i * columns + j] == 1)
			len++;
		else
			break;
	}
	if (len > max)
		max = len;

	return max;
}

int main(int argc, char *argv[])
{
	int arr[4][4] = {{0, 1, 1, 0},
					{0, 1, 1, 0},
					{0, 0, 0, 1},
					{0, 0, 0, 0}};
	int max, len, i, j;

	max = 0;
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; j++) {
			if (arr[i][j] == 1) {
				len = get_len(&arr[0][0], 4, 4, i, j);
				if (len > max)
					max = len;
			}
		}
	}

	printf("%d\n", max);

	return 0;
}
