int findInMatrix(int arr[], int rows, int columns, int value) {
	int row, column;

	row = 0;
	column = columns - 1;
	while (row < rows && column >= 0) {
		if (value < arr[row * columns + column])
			column--;
		else if (value > arr[row * columns + column])
			row++;
		else
			// found
			return 1;
	}

	return 0;
}
