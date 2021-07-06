/**
 * Read config file line by line
 * Sample Config file: test2.conf
 */

#include <stdio.h>
#include <string.h>

#define DEBUG

#define LINE_LENGTH_MAX     256
#define LINE_NUM_MAX        5

#define COMMENT_CHAR     '#'   /* Comment out character */
#define SEPERATOR_CHAR   '='   /* Key & Value seperator */

#define IS_START_BLANK_CHAR(c) \
			((c) == ' ' \
			 || (c) == '\t')

#define IS_END_BLANK_CHAR(c) \
			(c) == ' ' \
			 || (c) == '\t' \
			 || ((c) == '\r' \
			 || (c) == '\n')

#define IS_BLANK_LINE(c) \
			((c) == '\r' \
			 || (c) == '\n' \
			 || (c) == COMMENT_CHAR)

int read_config(char *pathname)
{
	FILE *fp = NULL;
	char linebuf[LINE_LENGTH_MAX] = {0};
	char *linep, *lineendp;
	char *key, *value;
	int lineno = 0;

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

		value = strchr(linep, SEPERATOR_CHAR);
		if (value == NULL)
			continue;
		*value++ = '\0';
		key = linep;

		printf(">>> key=%s, value=%s\n", key, value);
		
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
