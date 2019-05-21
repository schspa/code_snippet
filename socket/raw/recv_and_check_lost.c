#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>

#define BUF_SIZE 4096
#define SEND_LEN 1024

int main(int argc, char *argv[])
{
	int sockfd, ret;
	struct ifreq ifr;
	struct sockaddr_ll sl;

	/* RAW socket */
	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	/* Set NIC "eth0" as promisc mode */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	if (ret < 0) {
		perror("ioctl1");
		close(sockfd);
		return -1;
	}

	ifr.ifr_flags |= IFF_PROMISC;
	ret = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
	if (ret < 0) {
		perror("ioctl1");
		close(sockfd);
		return -1;
	}

	/* Define local network interface as eth0 */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ret = ioctl(sockfd, SIOCGIFINDEX, &ifr);
	if (ret < 0) {
		perror("ioctl3");
		close(sockfd);
		return -1;
	}

	memset(&sl, 0, sizeof(struct sockaddr_ll));
	sl.sll_ifindex = ifr.ifr_ifindex;
	sl.sll_family = PF_PACKET;
	sl.sll_protocol = htons(ETH_P_ALL);

	ret = bind(sockfd, (struct sockaddr *)&sl, sizeof(struct sockaddr_ll));
	if (ret < 0) {
		perror("bind");
		close(sockfd);
		return -1;
	}

	uint8_t buf[BUF_SIZE] = {0};
	uint8_t expect_char = 0;

	while (1) {
		if ((ret = recvfrom(sockfd, buf, BUF_SIZE, 0, NULL, NULL)) > 0) {

			if (ret <= 0) {
				perror("recvfrom");
				continue;
			}

			printf("Received %d bytes, data: 0x%x.\n", ret, buf[7]);

			/* Check whether packet lost */
			if (buf[7] != expect_char)
				fprintf(stderr, "Packet lost. buf[7]=0x%02x, expect=0x%02x\n", buf[7], expect_char);

			/* Update expected next character */
			if (buf[7] == 0xFF)
				expect_char = 0x06;
			else
				expect_char = buf[7] + 1;

			/* Send data back */
			//sendto(sockfd, buf, ret, 0, (struct sockaddr *)&sl, sizeof(struct sockaddr_ll));
		}
	}

	close(sockfd);

	return 0;
}
