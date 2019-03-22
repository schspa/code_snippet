/**
 * Read config file line by line.
 */

#include <stdio.h>
#include <string.h>

#define DEBUG

#define LINE_LENGTH_MAX     256
#define LINE_NUM_MAX        5

#define COMMENT_CHAR        '#'   /* Comment out character */
#define COLUMN_DELIMITER    ";"   /* Column seperater set string */

#define ISBLANKCHAR(c) \
			((c) == ' ' \
			 || (c) == '\t')
#define ISBLANKLINE(c) \
			((c) == '\r' \
			 || (c) == '\n' \
			 || (c) == COMMENT_CHAR)

int read_config(char *pathname)
{
	FILE *fp = NULL;
	char linebuf[LINE_LENGTH_MAX] = {0};
	char *linep, *wordp;
	int lineno = 0, columnno = 0;

	fp = fopen(pathname, "r");
	if (fp == NULL)
	{
		printf("Config file open failed.\n");
		return -1;
	}

	printf("\nReading data from config file.\n");

	while (lineno < LINE_NUM_MAX && fgets(linebuf, LINE_LENGTH_MAX, fp) != NULL)
	{
		linep = linebuf;

		while (ISBLANKCHAR(*linep))
			linep++;

		if (ISBLANKLINE(*linep))
			continue;
		
		printf("\n> Get data from config file line %d.\n", lineno);

		columnno = 0;
		while ((wordp = strsep(&linep, COLUMN_DELIMITER)) != NULL)
		{
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

#ifdef DEBUG
int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		printf("Usage: %s <config path>\n", argv[0]);
		return -1;
	}

	read_config(argv[1]);

	return 0;
}
#endif
