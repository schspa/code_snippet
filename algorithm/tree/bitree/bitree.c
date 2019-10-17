#include <stdlib.h>
#include "bitree.h"

bitnode_t *bitnode_alloc(int data) {
	bitnode_t *node = NULL;
	node = malloc(sizeof(bitnode_t));
	if (node == NULL)
		return node;
	node->data = data;
	node->left = NULL;
	node->right = NULL;
	return node;
}

int bitnode_free(bitnode_t *node) {
	if (node == NULL)
		return -1;
	free(node);
	return 0;
}

bitnode_t *bitree_init(int start, int end) {
	bitnode_t *root = NULL;

	if (start > end)
		return NULL;

	root = bitnode_alloc(start);
	if (root == NULL)
		return NULL;
	root->left = bitree_init(start * 2, end);
	root->right = bitree_init(start * 2 + 1, end);
	return root;
}

int bitree_destory(bitnode_t *root) {
	if (root == NULL)
		return -1;

	bitree_destory(root->left);
	bitree_destory(root->right);
	bitnode_free(root);
	return 0;
}

int bitree_preorder(bitnode_t *root, visit_t visit) {
	if (root == NULL)
		return -1;

	visit(root->data);
	bitree_preorder(root->left, visit);
	bitree_preorder(root->right, visit);
	return 0;
}

int bitree_inorder(bitnode_t *root, visit_t visit) {
	if (root == NULL)
		return -1;

	bitree_inorder(root->left, visit);
	visit(root->data);
	bitree_inorder(root->right, visit);
	return 0;
}

int bitree_postorder(bitnode_t *root, visit_t visit) {
	if (root == NULL)
		return -1;

	bitree_postorder(root->left, visit);
	bitree_postorder(root->right, visit);
	visit(root->data);
	return 0;
}

int bitree_getlevel(bitnode_t *root) {
	int leftlevel, rightlevel;

	if (root == NULL)
		return 0;

	leftlevel = bitree_getlevel(root->left);
	rightlevel = bitree_getlevel(root->right);

	return leftlevel > rightlevel ? leftlevel + 1 : rightlevel + 1;
}
