#ifndef __CONFIG_H__
#define __CONFIG_H__

#define COMMENT_CHAR     '#'   /* Comment out character */
#define SEPERATOR_CHAR   '='   /* Key & Value seperator */

#define IS_START_BLANK_CHAR(c) \
			 ((c) == ' ' \
			 || (c) == '\t')

#define IS_END_BLANK_CHAR(c) \
			 ((c) == ' ' \
			 || (c) == '\t' \
			 || (c) == '\r' \
			 || (c) == '\n')

#define IS_BLANK_LINE(c) \
			 ((c) == '\r' \
			 || (c) == '\n' \
			 || (c) == COMMENT_CHAR)

typedef void (*key_value_handler_t)(char *key, char *value, void *arg);

int parse_config(char *config, key_value_handler_t handler, void *arg);

#endif /* __CONFIG_H__ */
