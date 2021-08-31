#include <stdio.h>
#include <unistd.h>

#define OPTSTRING "g:s:h"

void usage(char *appname)
{
	printf("%s <opts>\n\n", appname);
	printf("opts can be:\n");
	printf("  -g <arg3>  xxxxx\n");
	printf("  -s <arg4>  xxxxx\n");
	printf("  -h         print this usage\n");
	printf("\n");
}

int main(int argc, char *argv[])
{
	int argflag = 0;

	while ((argflag = getopt(argc, (char **)argv, OPTSTRING)) != -1) {
		switch (argflag) {
		case 'g':
			printf("-g optarg=%s\n", optarg);
			break;
		case 's':
			printf("-s optarg=%s\n", optarg);
			break;
		case 'h':
		default:
			usage(argv[0]);
			return 0;
			break;
		}
	}

	return 0;
}
