#include <stdio.h>
#include "bitree.h"

int print_data(int data) {
	printf("%4d", data);
	return 0;
}

int main(int argc, char *argv[])
{
	bitnode_t *root = bitree_init(1, 10);

	printf("Preorder:\n");
	bitree_preorder(root, print_data);
	putchar('\n');
	printf("Inorder:\n");
	bitree_inorder(root, print_data);
	putchar('\n');
	printf("Postorder:\n");
	bitree_postorder(root, print_data);
	putchar('\n');

	printf("tree lelve = %d\n", bitree_getlevel(root));

	bitree_destory(root);
	return 0;
}
