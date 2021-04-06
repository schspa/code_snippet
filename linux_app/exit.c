#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void myexit(void)
{
	printf("Hello myexit.\n");
}

int main(int argc, char *argv[])
{
	atexit(myexit);

	printf("Hello main 1st.\n");

	printf("Hello main 2nd.");
	
	//exit(0);
	//_Exit(0);
	_exit(0);
	//return 0;
}
