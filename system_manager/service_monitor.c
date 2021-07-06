/**
 * Service Monitor
 *
 * This application is to monitor one service (from user specificied).
 * No matter the service exit normally or abnormally,
 * this application will pull up it again in a certain time.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syslog.h>

#define BUF_SIZE 512

int is_running(char *service)
{
	char cmd[BUF_SIZE] = {0}, buf[BUF_SIZE] = {0};
	FILE *fp = NULL;
	char *filename = NULL;

	sprintf(cmd, "ps -ef | grep %s | grep -v grep | awk '{print $8}'", service);
	fp = popen(cmd, "r");
	if (fp == NULL) {
		perror("popen");
		return -1;
	}

	while(fgets(buf, sizeof(buf), fp) != NULL) {
		filename = basename(buf);

		if(filename && strcmp(buf, service) == 0) {
			pclose(fp);
			return 1;
		}
	}
	pclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	openlog(argv[0], LOG_CONS | LOG_PID, LOG_USER);

	pid_t pid;
	int ret = 0;
	int status = 0;
	int i = 0;

	while(1) {
		pid = fork();
		if (pid < 0) {
			perror("fork failed");
			exit(1);
		} else if (pid == 0) {
			printf("\nChild Process: Start...\n\n");

			setenv("QTDIR", "/usr/local/QTE4.8.5", 1);
			setenv("LD_LIBRARY_PATH", "/usr/local/QTE4.8.5/lib", 1);
			setenv("QWS_DISPLAY", "LinuxFb:/dev/fb0", 1);

			ret = execl("/sda2/DCCU/dru", "dru", "-qws", "-display", "VNC:LinuxFb", NULL);
			if (ret < 0) {
				perror("execl failed");
				exit(21);
			}
		} else {
			if (wait(&status) != pid)
				perror("wait error");

			printf("\nParent Process: Child process completed, exit with %d\n\n", WEXITSTATUS(status));

			for (i = 5; i > 0; i--) {
				printf("i = %d\n", i);
				sleep(1);
			}
		}
	}

	closelog();

	return 0;
}
