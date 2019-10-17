#include <stdio.h>

#define MAX_BUF_SIZE 16

static int buf[MAX_BUF_SIZE] = {0};
static int top = 0;

int getch(void) {
	if (top > 0)
		return buf[--top];
	return getchar();
}

void ungetch(int ch) {
	if (top >= MAX_BUF_SIZE) {
		fprintf(stderr, "ungetch() error: too many characters in buffer.\n");
	} else {
		buf[top++] = ch;
	}
}
