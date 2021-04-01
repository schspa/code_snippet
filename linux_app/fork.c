#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int glob = 6;


int main(void)
{
	int var;
	pid_t pid;

	var = 88;

	printf("before fork\n");

	if ((pid = fork()) < 0) {
		printf("fork error");
		return 1;
	} else if (pid == 0) {      /* child */
		printf("child pid = %d\n", getpid());
		glob++;
		var++;
	} else {                    /* parent */
		printf("parent pid = %d\n", getpid());
		sleep(2);
	}

	printf("pid = %d, glob = %d, var = %d\n", getpid(), glob, var);

	exit(0);
}
