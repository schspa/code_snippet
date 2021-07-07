/**
 * Read config file line by line
 * Sample Config file: test2.conf
 */

#include <stdio.h>
#include <string.h>

#define DEBUG

#define LINE_LENGTH_MAX     256

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

typedef void (*key_value_handler_t)(char *key, char *value, void *arg);

int read_config(char *config, key_value_handler_t handler, void *arg)
{
	FILE *fp = NULL;
	char linebuf[LINE_LENGTH_MAX] = {0};
	char *linep, *lineendp;
	char *key, *value;
	int lineno = 0;

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
		
		printf("\n> Read line: lineno=%d, string=%s\n", lineno, linep);

		value = strchr(linep, SEPERATOR_CHAR);
		if (value == NULL)
			continue;
		*value++ = '\0';
		key = linep;

		handler(key, value, arg);
		
		lineno++;
		memset(linebuf, 0, LINE_LENGTH_MAX);
	}

	fclose(fp);

	return lineno;
}

#ifdef DEBUG

void handler_echo(char *key, char *value, void *arg)
{
	printf("%s: key=%s, value=%s\n", __func__, key, value);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <config path>\n", argv[0]);
		return -1;
	}

	read_config(argv[1], handler_echo, NULL);

	return 0;
}
#endif
