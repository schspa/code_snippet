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
#define ETH_RAW_XFER_TIMEOUT_US 1500


int main(int argc, char *argv[])
{
	int sockfd, ret;
	struct ifreq ifr;
	struct sockaddr_ll sl;
	struct timeval tv_out;

	char buf[BUF_SIZE] = {0};

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ret = ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	if (ret < 0) {
		perror("ioctl1");
		return -1;
	}

	ifr.ifr_flags |= IFF_PROMISC;
	ret = ioctl(sockfd, SIOCSIFFLAGS, &ifr);
	if (ret < 0) {
		perror("ioctl1");
		return -1;
	}

	tv_out.tv_sec = ETH_RAW_XFER_TIMEOUT_US / (1000 * 1000);
	tv_out.tv_usec = ETH_RAW_XFER_TIMEOUT_US - tv_out.tv_sec * 1000 * 1000;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv_out, sizeof(struct timeval));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv_out, sizeof(struct timeval));

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ret = ioctl(sockfd, SIOCGIFINDEX, &ifr);
	if (ret < 0) {
		perror("ioctl3");
		return -1;
	}

	memset(&sl, 0, sizeof(struct sockaddr_ll));
	sl.sll_ifindex = ifr.ifr_ifindex;
	sl.sll_family = PF_PACKET;
	sl.sll_protocol = htons(ETH_P_ALL);
	ret = bind(sockfd, (struct sockaddr *)&sl, sizeof(sl));
	if (ret < 0) {
		perror("bind");
	}

	struct timeval tmout;
	fd_set read_set;
	int ret2;

	while (1) {
		if ((ret = recvfrom(sockfd, buf, BUF_SIZE, 0, NULL, NULL)) > 0) {
			printf("%x%x %x\n", buf[0], buf[1], buf[7]);
			sendto(sockfd, buf, ret, 0, (struct sockaddr *)&sl, sizeof(struct sockaddr_ll));
		}
	}

	close(sockfd);

	return 0;
}
