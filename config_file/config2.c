/**
 * Read config file line by line
 * Sample Config file: test1.conf
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config2.h"

#define LINE_LENGTH_MAX     256

int read_config(char *config)
{
	FILE *fp = NULL;
	char linebuf[LINE_LENGTH_MAX] = {0};
	char *linep, *lineendp, *wordp;
	int lineno = 0, columnno = 0;

	fp = fopen(config, "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}

	while (fgets(linebuf, LINE_LENGTH_MAX, fp) != NULL) {
		linep = linebuf;

		/* Strip start of line */
		while (IS_START_BLANK_CHAR(*linep))
			linep++;

		/* Strip blank line */
		if (IS_BLANK_LINE(*linep))
			continue;

		/* Strip end of line */
		lineendp = linep + strlen(linep) - 1;
		while (IS_END_BLANK_CHAR(*lineendp)) {
			*lineendp = '\0';
			lineendp--;
		}

		printf("\n> Get data from config file line num=%d, string=%s\n", lineno, linep);

		columnno = 0;
		while ((wordp = strsep(&linep, COLUMN_SEP)) != NULL) {
			printf(">>> Column %2d: %s [0x%2x]\n", columnno, wordp, *wordp);
			columnno++;
		}
		
		lineno++;
		memset(linebuf, 0, LINE_LENGTH_MAX);
	}

	printf("\n> Config file read finished.\n");
	fclose(fp);

	return lineno;
}

#define DEBUG
#ifdef DEBUG
int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <config path>\n", argv[0]);
		return -1;
	}

	read_config(argv[1]);

	return 0;
}
#endif
