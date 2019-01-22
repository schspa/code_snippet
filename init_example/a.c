#include <stdio.h>
#include "init.h"

static void a_init(void)
{
	printf("a init is called.\n");
}
DECLARE_INIT(a_init);
