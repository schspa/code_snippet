#include <stdio.h>
#include "init.h"

static void b_init(void)
{
	printf("b init is called.\n");
}
DECLARE_INIT(b_init);
