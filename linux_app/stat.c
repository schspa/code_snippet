#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	int i;
	struct stat buf;
	char *ptr;

	if (argc < 2) {
		printf("Usage: %s <file1> [file2] ...\n", argv[0]);
		return -1;
	}

	for (i = 1; i < argc; i++) {
		printf("%s: ", argv[i]);
		if (lstat(argv[i], &buf) < 0) {
			perror("lstat error");
			continue;
		}

		if (S_ISREG(buf.st_mode))
			ptr = "Regular file";
		else if (S_ISDIR(buf.st_mode))
			ptr = "Directory";
		else if (S_ISCHR(buf.st_mode))
			ptr = "Character device";
		else if (S_ISBLK(buf.st_mode))
			ptr = "Block device";
		else if (S_ISFIFO(buf.st_mode))
			ptr = "Fifo";
		else if (S_ISLNK(buf.st_mode))
			ptr = "Symbolic link";
		else if (S_ISSOCK(buf.st_mode))
			ptr = "Socket";
		else
			ptr = "** unknown **";

		printf("%s\n", ptr);
	}

	return 0;
}
