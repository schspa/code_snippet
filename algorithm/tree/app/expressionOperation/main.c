#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "stack.h"
#include "bitree.h"

#define NUMBER 0

static int printf_data(int data) {
	if (data == '+' || data == '-' || data == '*' || data == '/' || data == '%')
		printf("%3c", data);
	else
		printf("%3d", data);
	return 0;
}

static int getop(char s[]) {
	int c, i;

	while ((s[0] = c = getchar()) == ' ' || c == '\t')
		;

	if (!isdigit(c)) {
		s[1] = '\0';
		return c;
	}

	i = 0;
	while (isdigit(s[++i] = c = getchar()))
		;
	s[i] = '\0';
	if (c != EOF)
		ungetc(c, stdin);
	return NUMBER;
}

static void inorder_fn(bitnode_t *root, visit_t visit) {
	if (root->left == NULL && root->right == NULL) {
		visit(root->data);
		return;
	}


	putchar('(');
	inorder_fn(root->left, visit);
	putchar(')');
	
	visit(root->data);

	putchar('(');
	inorder_fn(root->right, visit);
	putchar(')');
}

static void inorder(bitnode_t *root) {
	printf("Inorder:\n");
	inorder_fn(root, printf_data);
	putchar('\n');
}

static void postorder(bitnode_t *root) {
	printf("Postorder:\n");
	bitree_postorder(root, printf_data);
	putchar('\n');
}

static int caculate(bitnode_t *root) {
	if (root->left == NULL && root->right == NULL)
		return root->data;

	int left, right;
	left = caculate(root->left);
	right = caculate(root->right);
	switch (root->data) {
	case '+':
		return left + right;
		break;
	case '-':
		return left - right;
		break;
	case '*':
		return left * right;
		break;
	case '/':
		return left / right;
		break;
	case '%':
		return left % right;
		break;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char s[8] = {0};
	int ch, n;
	bitnode_t *node, *root[8];

	n = 0;
	while ((ch = getop(s)) != EOF) {
		switch(ch) {
		case NUMBER:
			node = bitnode_alloc(atoi(s));
			if (node != NULL)
				push(node);
			else
				fprintf(stderr, "node_alloc() error.\n");
			break;
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
			node = bitnode_alloc(ch);
			if (node != NULL) {
				node->right = pop();
				node->left = pop();
				push(node);
			} else {
				fprintf(stderr, "node_alloc() error.\n");
			}
			break;
		case '\n':
			root[n++] = pop();
			break;
		}
	}

	for (int i = 0; i < n; i++) {
		printf("********** tree %d*************\n", i);
		postorder(root[i]);
		inorder(root[i]);
		printf("result = %d\n\n", caculate(root[i]));
		bitree_destory(root[i]);
	}

	return 0;
}
