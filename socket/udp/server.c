#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "inc.h"

#define BUF_SIZE  1024
#define IP_LISTEN IP_ANY

int main(int argc, char *argv[])
{
	int sockfd, ret;

	/* UDP Socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

	printf("Now UDP server listen at host:%s port:%d\n", IP_LISTEN, PORT);

	struct sockaddr_in clientaddr;
	socklen_t clientlen;
	char clientaddr_str[16];
	uint16_t clientport;

	char buf[BUF_SIZE] = {0};

	while(1) {
		/* Received from client */
		memset(buf, 0, sizeof(buf));
		
		/* <UNIX环境高级编程> Ch3.3, 4.6, 8.2: Value-Result参数 */
		clientlen = sizeof(clientaddr);
		ret = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&clientaddr, &clientlen);
		if (ret < 0) {
			perror("recv");
			continue;
		}

		memset(clientaddr_str, 0, sizeof(clientaddr_str));
		strncpy(clientaddr_str, inet_ntoa(clientaddr.sin_addr), sizeof(clientaddr_str));
		clientport = ntohs(clientaddr.sin_port);
		printf("<<<[%s:%d] %d bytes recevied: %s\n", clientaddr_str, clientport, ret, buf);

		if (strncmp(buf, "quitall", 4) == 0
				|| strncmp(buf, "exitall", 4) == 0)
			break;

		/* Send back to client */
		ret = sendto(sockfd, &buf, strlen(buf), 0, (struct sockaddr *)&clientaddr, sizeof(struct sockaddr));
		if (ret < 0) {
			perror("send");
			break;
		}
		printf(">>>[%s:%d] %d bytes sent back.\n", clientaddr_str, clientport, ret);
	}

	close(sockfd);
	return 0;

err:
	close(sockfd);
	return -1;
}
