#include <stdio.h>
#include "single_linklist.h"

#define __TEST__
#ifdef __TEST__
int print_data(int data) {
	printf("%d ", data);
	return 0;
}

int main(int argc, char *argv[])
{
	int i = 0;
	list_t *list = NULL;
	
	list = list_init(10);

	for (i = 10; i > 0; i--) {
		list_insert(list, i);
	}

	list_foreach(list, print_data);
	putchar('\n');

	list_modify(list, 5, 50);

	node_t *node = list_search(list, 8);
	node->data = 88;

	list_delete(list, 1);

	list_foreach(list, print_data);
	putchar('\n');

	list_reverse(list);

	list_foreach(list, print_data);
	putchar('\n');

	list_destory(list);
	return 0;
}
#endif /* __TEST__ */
