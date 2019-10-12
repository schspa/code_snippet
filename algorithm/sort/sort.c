void bubble_sort(int arr[], int n) {
	if (n < 1)
		return;

	int i, j;
	int temp, flag = 0;

	for (i = 0; i < n; i++) {
		flag = 0;
		for (j = 0; j < n - i - 1; j++) {
			if (arr[j] > arr[j + 1]) {
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
				flag = 1;
			}
		}

		if (flag == 0)
			break;
	}
}

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
