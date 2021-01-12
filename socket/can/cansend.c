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
#include <linux/can/error.h>

#define FRAME_TYPE_NUM      4
#define FRAME_TYPE_NAME_LEN 16

#define FRAME_TYPE_STD_IDX 0
#define FRAME_TYPE_EFF_IDX 1
#define FRAME_TYPE_RTR_IDX 2
#define FRAME_TYPE_ERR_IDX 3

#define SEND_FRAME_NUM 3

static const char frame_type_name[FRAME_TYPE_NUM][FRAME_TYPE_NAME_LEN] = {
	"Standard Frame",
	"Extend Frame",
	"Remote Frame",
	"Error Frame",
};

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("\nUsage: %s <can port>\n\n", argv[0]);
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

	/* Disable frame filter rule, we don't receive CAN frame */
	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	/* Construct 2 frames ready to send */
	struct can_frame frame[SEND_FRAME_NUM];
	frame[0].can_id = 0x12;                    /* CAN ID: Standard frame */
	frame[0].can_dlc = 2;                      /* Data length */
	frame[0].data[0] = 0x01;                   /* Data content */
	frame[0].data[1] = 0xAB;
	frame[1].can_id = CAN_EFF_FLAG | 0x12;     /* CAN ID: Extend frame */
	frame[1].can_dlc = 2;                      /* Data length */
	frame[1].data[0] = 0x02;                   /* Data content */
	frame[1].data[1] = 0xCD;
	frame[2].can_id = CAN_RTR_FLAG | 0x12;     /* CAN ID: Remote frame */
	frame[2].can_dlc = 0;                      /* Data length */
	/* Error Frame is generated from system, we cant send Error frame from app. */
	//frame[3].can_id = CAN_ERR_FLAG | CAN_ERR_CRTL;     /* CAN ID: Error frame */
	//frame[3].can_dlc = 0;                              /* Data length */

	int i = 0;
	int cycle = 0;
	int frame_type_idx = 0;
	while(cycle++ < 1) {
		/* Only one frame can be sent at a time */
		for (i = 0; i < SEND_FRAME_NUM; ++i) {
			ret = send(sockfd, &frame[i], sizeof(frame[i]), 0);
			if (ret <= 0) {
				perror("send frame");
				break;
			}

			if (frame[i].can_id & CAN_EFF_FLAG) {
				frame_type_idx = FRAME_TYPE_EFF_IDX;
			} else if (frame[i].can_id & CAN_RTR_FLAG) {
				frame_type_idx = FRAME_TYPE_RTR_IDX;
			} else if (frame[i].can_id & CAN_ERR_FLAG) {
				frame_type_idx = FRAME_TYPE_ERR_IDX;
			} else {
				frame_type_idx = FRAME_TYPE_STD_IDX;
			}
			printf("[cycle=%d] send Frame %d (%s) success.\n", cycle, i, frame_type_name[frame_type_idx]);
			sleep(1);
		}
	}

	close(sockfd);

	return 0;
}
