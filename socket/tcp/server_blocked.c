#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "inc.h"

#define BUF_SIZE  1024
#define IP_LISTEN IP_LOOPBACK

int main(int argc, char *argv[])
{
	int sockfd, ret;

	/* TCP Socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	/* Set IP and PORT to BIND */
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(IP_LISTEN);

	/* Eliminates "Address already in use" error form bind. <Computer System 2nd> P629 */
	int on = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		perror("setsockopt");
		goto err;
	}

	/* Bind Server IP and PORT */
	ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
	if (ret < 0) {
		perror("bind");
		goto err;
	}

	printf("Now TCP server listen at host:%s port:%d\n", IP_LISTEN, PORT);
	ret = listen(sockfd, 1024);
	if (ret < 0) {
		perror("listen");
		goto err;
	}

	int i = 0;
	int cycle = 0;
	char buf[BUF_SIZE] = {0};

	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	int clientfd = 0;

	/* Only one client request can be processed at the same time */
	while(1) {
		clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &clientlen);
		if (clientfd < 0) {
			perror("accept");
			continue;
		}

		printf(">>> Accept a new TCP client from host:%s port:%d\n", IP_LISTEN, PORT);
		for (cycle = 0; cycle < 3; cycle++) {
			ret = recv(clientfd, buf, BUF_SIZE, 0);
			if (ret < 0) {
				perror("recv");
				close(clientfd);
				continue;
			}

			printf("------------------cycle=%d------------------\n", cycle);
			for (i = 0; i < ret; i++) {
				putchar(buf[i]);
			}
			putchar('\n');
		}
		close(clientfd);
	}

	close(sockfd);
	return 0;

err:
	close(sockfd);
	return -1;
}
