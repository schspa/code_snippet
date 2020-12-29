#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#define IF_NAME   "can0"

int main(int argc, char *argv[])
{
	int sockfd, ret;
	struct sockaddr_can addr;
	struct ifreq ifr;

	/* CAN socket */
	sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	/* Set CAN device "can0" */
	strcpy(ifr.ifr_name, IF_NAME);
	ret = ioctl(sockfd, SIOCGIFINDEX, &ifr);
	if (ret < 0) {
		perror("ioctl");
		close(sockfd);
		return -1;
	}

	/* Bind can0 to sockfd */
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));

	/* Disable frame filter rule, we don't receive CAN frame */
	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	/* Construct 2 frames ready to send */
	struct can_frame frame[2];
	frame[0].can_id = 0x12;                    /* CAN ID: Standard frame */
	frame[0].can_dlc = 2;                       /* Data length */
	frame[0].data[0] = 0x01;                    /* Data content */
	frame[0].data[1] = 0xAB;
	frame[1].can_id = CAN_EFF_FLAG | 0x12;     /* CAN ID: Extend frame */
	frame[1].can_dlc = 2;                       /* Data length */
	frame[1].data[0] = 0x02;                    /* Data content */
	frame[1].data[1] = 0xCD;                    /* Data content */

	int cycle = 0;
	while(cycle++ < 5) {
		/* Only one frame can be sent at a time */
		ret = send(sockfd, &frame[0], sizeof(frame[0]), 0);
		if (ret <= 0) {
			perror("send frame 0");
			break;
		}
		printf("[cycle=%d] send Standard Frame 0 success.\n", cycle);
		sleep(1);

		ret = send(sockfd, &frame[1], sizeof(frame[1]), 0);
		if (ret <= 0) {
			perror("send frame 1");
			break;
		}
		printf("[cycle=%d] send Extend Frame 1 success.\n", cycle);
		sleep(1);
	}

	close(sockfd);

	return 0;
}
