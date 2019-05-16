#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 16

int main(int argc, char *argv[])
{
	int sockfd, ret;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8888);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int on = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
	if (ret < 0) {
		perror("bind");
		close(sockfd);
		return -1;
	}

	uint8_t expect_char = 0, buf[BUF_SIZE] = {0};

	while(1) {
		ret = recv(sockfd, buf, BUF_SIZE, 0);
		if (ret < 0) {
			perror("recv");
			continue;
		}

		if (expect_char != buf[0])
			fprintf(stderr, "Packet lost, expected: 0x%02x, received: 0x%02x\n", expect_char, buf[0]);
	}

	return 0;
}
