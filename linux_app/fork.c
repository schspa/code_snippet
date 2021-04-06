#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int glob = 6;


int main(void)
{
	int var = 88;
	pid_t pid;

	printf("before fork, getpid()=%d\n", getpid());

	if ((pid = fork()) < 0) {   /* error */
		printf("fork error");
		return 1;
	} else if (pid == 0) {      /* child process */
		printf("After fork, in child process getpid()=%d, pid=%d\n", getpid(), pid);
		glob++;
		var++;
	} else {                    /* parent process */
		printf("After fork, in parent process getpid()=%d, pid=%d\n", getpid(), pid);
		sleep(2);
	}

	/* This statement will be executed in both parent and child processes */
	printf("pid = %d, glob = %d, var = %d\n", getpid(), glob, var);

	exit(0);
}
