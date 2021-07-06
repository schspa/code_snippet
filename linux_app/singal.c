#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

void fun(int sig)
{
	if(sig == SIGQUIT) {
		printf("Received SIGQUIT signal.\n");
	} else if(sig == SIGUSR1) {
		printf("Received SIGUSR1 signal.\n");
	} else if(sig == SIGUSR2) {
		printf("Received SIGUSR2 signal.\n");
	}
}

int main(int argc, const char *argv[])
{
	signal(SIGINT, SIG_IGN);  /* kill  -2 xxx, Ctl + C, Ignore */
	signal(SIGTERM, SIG_DFL); /* kill -15 xxx, kill xxx, default behavior */
	signal(SIGQUIT, fun);     /* kill  -3 xxx */
	signal(SIGUSR1, fun);     /* kill -10 xxx */
	signal(SIGUSR2, fun);     /* kill -12 xxx */

	printf("Main start.\n");
	while (1) {
		sleep(1);
	}

	return 0;
}

