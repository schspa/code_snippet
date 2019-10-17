#include <stdio.h>

#define MAX_BUF_SIZE 32

static int buf[MAX_BUF_SIZE] = {0};
static int top = 0;

void push(int data) {
	if (top >= MAX_BUF_SIZE) {
		fprintf(stderr, "push() error: stack full!\n");
		return;
	}
	buf[top++] = data;
}

int pop() {
	if (top <= 0) {
		fprintf(stderr, "pop() error: stack empty!\n");
		return 0;;
	}
	return buf[--top];
}
