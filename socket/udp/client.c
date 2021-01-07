#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "inc.h"

#define BUF_SIZE 64

int main(int argc, char *argv[])
{
	int sockfd, ret;

	/* UDP Socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(IP_LOOPBACK);

	/* Connect to UDP Server */
	ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
	if (ret < 0) {
		perror("connect");
		close(sockfd);
		return -1;
	}

	int cycle = 0;
	char buf[BUF_SIZE] = {0};

	while(getchar() != 'q') {
		sprintf(buf, "%d-%s", cycle, "Hello World");
		ret = send(sockfd, &buf, strlen(buf)+1, 0);
		if (ret < 0) {
			perror("send");
		}
		printf("Send %d bytes, data: %s.\n", ret, buf);
		cycle++;
	}

	close(sockfd);
	return 0;
}
