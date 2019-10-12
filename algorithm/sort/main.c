#define __TEST__
#ifdef  __TEST__

#include <stdio.h>
#include "sort.h"

void print_arr(int arr[], int n) {
	int i = 0;

	for (i = 0; i < n; i++) {
		printf("%d ", arr[i]);
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	int arr[9] = {1, 3, 5, 3, 5, 1, 2, 9, 3};

	print_arr(arr, 9);

	bubble_sort(arr, 9);

	print_arr(arr, 9);

	insert_sort(arr, 9);
	
	print_arr(arr, 9);
	
	return 0;
}

#endif /* __TEST__ */
