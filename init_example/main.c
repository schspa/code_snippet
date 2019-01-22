#include "init.h"

static void callinit(void)
{
	init_fn_t *fn;

	for (fn = &__start_init_sec; fn < &__stop_init_sec; fn++)
		(*fn)();
}

int main(int argc, char *argv[])
{
	callinit();

	return 0;
}
