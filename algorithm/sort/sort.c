#include <stdio.h>

static void swap(int arr[], int i, int j) {
	int tmp;
	tmp = arr[i];
	arr[i] = arr[j];
	arr[j] = tmp;
}

/**********************************************/

void bubble_sort(int arr[], int n) {
	if (n < 1)
		return;

	int i, j;
	int temp, flag = 0;

	for (i = 0; i < n; i++) {
		flag = 0;
		for (j = 0; j < n - i - 1; j++) {
			if (arr[j] > arr[j + 1]) {
				swap(arr, j, j + 1);
				flag = 1;
			}
		}

		if (flag == 0)
			break;
	}
}

/****************************************************/

void insert_sort(int arr[], int n) {
	int i, j;
	int temp;

	for (i = 1; i < n; i++) {
		temp = arr[i];

		for (j = i; j > 0 && temp < arr[j - 1]; j--)
			arr[j] = arr[j - 1];

		arr[j] = temp;
	}
}

/****************************************************/

static int partition(int arr[], int low, int high) {
	if (arr == NULL || low >= high)
		return -1;

	int pivotkey;
	pivotkey = arr[low];
	while (low < high) {
		while (low < high && arr[high] >= pivotkey)
			high--;
		if (low < high)
			arr[low++] = arr[high];

		while (low < high && arr[low] <= pivotkey)
			low++;
		if (low < high)
			arr[high--] = arr[low];
	}
	/* 此时low等于high */
	arr[low] = pivotkey;
	return low;
}

static void qsort(int arr[], int low, int high) {
	if (low >= high)
		return;

	int pivot;
	pivot = partition(arr, low, high);
	qsort(arr, low, pivot - 1);
	qsort(arr, pivot + 1, high);
}

void quick_sort(int arr[], int n) {
	qsort(arr, 0, n - 1);
}
