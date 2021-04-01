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

	/* Set Server IP and Port */
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(IP_LOOPBACK);

	struct sockaddr_in remoteaddr;
	//socklen_t remotelen = sizeof(struct sockaddr_in);
	socklen_t remotelen;
	char remoteaddr_str[16];
	uint16_t remoteport;

	char send_buf[BUF_SIZE] = {0};
	char recv_buf[BUF_SIZE] = {0};

	while(1) {
		memset(send_buf, 0, sizeof(send_buf));
		fgets(send_buf, sizeof(send_buf), stdin);

		ret = sendto(sockfd, &send_buf, strlen(send_buf), 0, (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
		if (ret < 0) {
			perror("send");
			break;
		}
		printf(">>>[%s:%d] %d bytes sent to server.\n", IP_LOOPBACK, PORT, ret);

		if (strncmp(send_buf, "quit", 4) == 0
				|| strncmp(send_buf, "exit", 4) == 0)
			break;

		memset(recv_buf, 0, sizeof(recv_buf));

		/* <UNIX环境高级编程> Ch3.3, 4.6, 8.2: Value-Result参数 */
		remotelen = sizeof(remoteaddr);
		ret = recvfrom(sockfd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&remoteaddr, &remotelen);
		if (ret < 0) {
			perror("recv");
			break;
		}

		memset(remoteaddr_str, 0, sizeof(remoteaddr_str));
		strncpy(remoteaddr_str, inet_ntoa(remoteaddr.sin_addr), sizeof(remoteaddr_str));
		remoteport = ntohs(remoteaddr.sin_port);
		printf("<<<[%s:%d] %d bytes recevied: %s\n", remoteaddr_str, remoteport, ret, recv_buf);
	}

	close(sockfd);
	return 0;
}
