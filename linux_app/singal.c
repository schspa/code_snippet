#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

void fun(int sig)
{
	if(sig == SIGUSR1) {
		printf("Received SIGUSR1 signal.\n");
	} else if(sig == SIGUSR2) {
		printf("Received SIGUSR2 signal.\n");
	}
}

int main(int argc, const char *argv[])
{
	signal(SIGUSR1, fun);
	signal(SIGUSR2, fun);

	printf("Main start.\n");
	while (1) {
		sleep(1);
	}

	return 0;
}

