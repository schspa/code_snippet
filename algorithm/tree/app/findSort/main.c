#include <stdio.h>
#include "bitree.h"

int print_data(int data) {
	printf("%3d", data);
	return 0;
}

void preorder(bitnode_t *root) {
	printf("Preorder:\n");
	bitree_preorder(root, print_data);
	putchar('\n');
}

void inorder(bitnode_t *root) {
	printf("Inorder:\n");
	bitree_inorder(root, print_data);
	putchar('\n');
}

void postorder(bitnode_t *root) {
	printf("Postorder:\n");
	bitree_postorder(root, print_data);
	putchar('\n');
}

bitnode_t *find(bitnode_t *root, int data) {
	if (root == NULL)
		return NULL;

	if (data == root->data)
		return root;
	if (data < root->data)
		return find(root->left, data);
	return find(root->right, data);
}

void sort(bitnode_t *root) {
	printf("Sort:\n");
	bitree_inorder(root, print_data);
	putchar('\n');
}

bitnode_t *insert_node(bitnode_t *root, int data) {
	if (root == NULL)
		root = bitnode_alloc(data);
	else if (data < root->data)
		root->left = insert_node(root->left, data);
	else if (data > root->data)
		root->right = insert_node(root->right, data);
	/* else: the data is in the tree already, do nothing. */
	return root;
}

int main(int argc, char *argv[])
{
	int n = 0, data, i;
	bitnode_t *root = NULL;

	scanf("%d", &n);
	for (i = 0; i < n; ++i) {
		scanf("%d", &data);
		root = insert_node(root, data);
	}

	preorder(root);
	inorder(root);
	postorder(root);

	sort(root);

	int f;
	scanf("%d", &f);
	bitnode_t *tmp = find(root, f);
	if (tmp == NULL)
		printf("number %d didn't found.\n", f);
	else
		printf("number %d found.\n", tmp->data);

	bitree_destory(root);
	return 0;
}
