#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <stdio.h>
#include <errno.h>
#include <net/if.h>

struct MacHookTable {
	char ifr_name[IFNAMSIZ]; /* Interface name */
	struct sockaddr ifr_hwaddr;
};

int parse_config_file(struct MacHookTable *table, int len);


#define THERMAL_CONFIG_FILE_PATH "/etc/hook.json"
#endif
