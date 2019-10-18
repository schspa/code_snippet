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

static int partition(int arr[], int start, int end) {
	int i, j, pivot;

	pivot = arr[end];
	i = start;
	for (j = i; j < end; j++) {
		if (arr[j] < pivot) {
			swap(arr, j, i);
			i++;
		}
	}
	swap(arr, i, end);
	return i;
}

static void qsort(int arr[], int start, int end) {
	if (start >= end)
		return;

	int pivot;
	pivot = partition(arr, start, end);
	qsort(arr, start, pivot - 1);
	qsort(arr, pivot + 1, end);
}

void quick_sort(int arr[], int n) {
	qsort(arr, 0, n - 1);
}
