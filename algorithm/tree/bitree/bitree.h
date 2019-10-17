#ifndef __BIT_TREE_H__
#define __BIT_TREE_H__

typedef struct _bitnode_t_ {
	int data;
	struct _bitnode_t_ *left;
	struct _bitnode_t_ *right;
} bitnode_t;

bitnode_t *bitnode_alloc(int data);
int bitnode_free(bitnode_t *node);

bitnode_t *bitree_init(int start, int end);
int bitree_destory(bitnode_t *root);

typedef int (*visit_t)(int data);
int bitree_preorder(bitnode_t *root, visit_t visit);
int bitree_inorder(bitnode_t *root, visit_t visit);
int bitree_postorder(bitnode_t *root, visit_t visit);
int bitree_getlevel(bitnode_t *root);

#endif /* __BIT_TREE_H__ */
