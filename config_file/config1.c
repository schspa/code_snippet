#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config1.h"

#define LINE_BUF_SIZE 512

int parse_config(char *cfg, key_value_handler_t handler, void *arg)
{
	FILE *fp = NULL;
	char linebuf[LINE_BUF_SIZE] = {0};
	char *linep, *lineendp;
	char *key, *value;

	fp = fopen(cfg, "r");
	if (fp == NULL) {
		perror("fopen");
		return -1;
	}

	while (fgets(linebuf, LINE_BUF_SIZE, fp) != NULL) {
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
		
		value = strchr(linep, SEPERATOR_CHAR);
		if (value == NULL)
			continue;
		*value++ = '\0';
		key = linep;

		handler(key, value, arg);
		memset(linebuf, 0, LINE_BUF_SIZE);
	}

	fclose(fp);
	return 0;
}


#define __DEBUG__
#ifdef __DEBUG__

#define QTE_ENV_KEY_SKIP     "export"

void echo(char *key, char *value, void *arg)
{
	if (strncmp(key, QTE_ENV_KEY_SKIP, strlen(QTE_ENV_KEY_SKIP)) == 0)
		key += strlen(QTE_ENV_KEY_SKIP);

	while (IS_START_BLANK_CHAR(*key))
		key++;

	printf("key=%s, value=%s\n", key, value);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <config path>\n", argv[0]);
		return -1;
	}

	parse_config(argv[1], echo, NULL);
	return 0;
}

#endif
