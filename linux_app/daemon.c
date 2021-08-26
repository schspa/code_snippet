/**
 * Create Daemon Process
 * Reference：<UNIX环境高级编程> Chapter13
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	/* 1. Fork to create new process, and have parent process exit */
	pid_t pid = fork();
	if(pid < 0) {
		perror("fork");
		exit(-1);
	} else if(pid > 0) {
		exit(0);
	}

	/* 2. Create new session, set the process becomes the leader of the new session */
	setsid();

	/* 3. Change the current working directory to the root directory  */
	chdir("/");

	/* 4. Set the file mode creation mask to 0 */
	umask(0);

	/* 5. Close all openned fd */
	for(int i = 0; i < getdtablesize(); i++) {
		close(i);
	}

	while(1) {
		/* do daemon things */
	}

	return 0;
}
