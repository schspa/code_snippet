#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
	pid_t pid;
	int ret = 0;
	int status = 0;

	pid = fork();
	if (pid < 0) {
		perror("fork failed");
		exit(1);
	} else if (pid == 0) {
		printf("\nChild Process: execute \'ls\' command\n\n");
#if 0
		ret = execl("/bin/ls", "ls", "-l", "--color", NULL);
#else
		char *argvec[5] = {
			"ls",
			"aaa",
			"bbb",
			"ccc",
			NULL
		};
		ret = execv("/bin/ls", argvec);
#endif
		if (ret < 0) {
			perror("execl failed");
			exit(21);
		}
		/* Will never be executed here */
		exit(20);
	} else {
		if (wait(&status) != pid)
			perror("wait error");
		printf("\nParent Process: Child \'ls\' completed, exit with %d\n\n", WEXITSTATUS(status));
		exit(0);
	}

	return 0;
}
