#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t pid;
	int status = 0;

	pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	} else if (pid == 0) {
		printf("\nChild Process: execute \'ls\' command\n\n");
		execl("/bin/ls", "ls", "-l", "--color", NULL);
		perror("execl failed");
		exit(21);
	} else {
		if (wait(&status) != pid)
			perror("wait error");
		printf("\nParent Process: Child \'ls\' completed, exit with %d\n\n", WEXITSTATUS(status));
		exit(0);
	}

	return 0;
}
