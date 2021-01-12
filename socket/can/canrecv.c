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

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("\nUsage: %s <can port> [can id]\n\n", argv[0]);
		return -1;
	}

	int sockfd, ret;
	struct sockaddr_can addr;
	struct ifreq ifr;

	/* CAN socket */
	sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (sockfd < 0) {
		perror("socket");
		return -1;
	}

	/* Set CAN port getting from cmd line argument */
	strcpy(ifr.ifr_name, argv[1]);
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

	/* Add frame filter rule to get the wanted frame */
	if (argc >= 3) {
		struct can_filter filter[1];
		int can_id = strtol(argv[2], NULL, 16);
		filter[0].can_id = can_id;
		filter[0].can_mask = CAN_ERR_MASK;
		setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));
		printf("Add CAN frame filter: can_id=0x%x\n", can_id);
	}

	/* CAN frame */
	struct can_frame frame;

	int cycle = 0;
	while(1) {
		ret = recv(sockfd, &frame, sizeof(frame), 0);
		if (ret <= 0) {
			perror("recv");
			continue;
		}

		printf("[cycle=%d] ID=0x%x DLC=0x%x data[0]=0x%x data[1]=0x%x\n",
					cycle++, frame.can_id, frame.can_dlc, frame.data[0], frame.data[1]);

		if (frame.can_id & CAN_EFF_FLAG) {
			printf("- Extend Frame\n");
		} else if (frame.can_id & CAN_RTR_FLAG) {
			printf("- Remote Frame\n");
		} else if (frame.can_id & CAN_ERR_FLAG) {
			printf("- Error Frame\n");
		} else {
			printf("- Standard Frame\n");
		}

		sleep(1);
	}

	close(sockfd);

	return 0;
}
