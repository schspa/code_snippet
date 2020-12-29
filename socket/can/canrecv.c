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

/* To enable/disable the frame filter */
#undef FRAME_FILTER

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

#ifdef FRAME_FILTER
	/* Add frame filter rule to get the wanted frame */
	struct can_filter filter[1];
	filter[0].can_id = 0x11;
	filter[0].can_mask = C_SFF_MASK;
	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, &filter, sizeof(filter));
#endif

	/* CAN frame */
	struct can_frame frame;

	int cycle = 0;
	while(cycle++ < 5) {
		ret = recv(sockfd, &frame, sizeof(frame), 0);
		if (ret <= 0) {
			perror("recv");
			continue;
		}

		printf("[cycle=%d] ID=0x%x DLC=%d data[0]=0x%x data[1]=0x%x\n",
					cycle, frame.can_id, frame.can_dlc, frame.data[0], frame.data[1]);

		sleep(1);
	}

	close(sockfd);

	return 0;
}
