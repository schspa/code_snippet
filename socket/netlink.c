#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <linux/netlink.h>

static void die(char *s)
{
	write(STDOUT_FILENO, s, strlen(s));
	exit(1);
}

int main(int argc, char *argv[])
{
	struct sockaddr_nl nls;
	struct pollfd pfd;
	char buf[512];

	// Open hotplug event netlink socket
	memset(&nls, 0, sizeof(struct sockaddr_nl));
	nls.nl_family = AF_NETLINK;
	nls.nl_pid = getpid();
	nls.nl_groups = -1;

	pfd.events = POLLIN;
	pfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
	if (pfd.fd == -1)
		die("Not root\n");

	// Listen to netlink socket
	if (bind(pfd.fd, (void *)&nls, sizeof(struct sockaddr_nl)))
		die("Bind failed\n");

	int i, len;
	while (-1 != poll(&pfd, 1, -1)) {
		memset(buf, 0, sizeof(buf));

		len = recv(pfd.fd, buf, sizeof(buf), MSG_DONTWAIT);
		if (len == -1)
			die("Recv\n");

		// Print the data to stdout
		i = 0;
		while (i < len) {
			printf("%s\n", buf + i);
			i += strlen(buf + i) + 1;
		}
	}

	die("Poll\n");

	// Dear gcc: shut up.
	return 0;
}
