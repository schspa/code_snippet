#include <stdio.h>

int main(int argc, char *argv[])
{
	int i = 0;
	extern char **environ;

	while (*environ != NULL)
	{
		printf("[%d]%s\n", i, *environ);
		environ++;
		i++;
	}

	return 0;
}
