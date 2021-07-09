#ifndef __CONFIG2_H__
#define __CONFIG2_H__

#define COMMENT_CHAR        '#'   /* Comment out character */
#define COLUMN_SEP          ";"   /* Column seperater set string */
#define COLUMN_SEP_CHAR   (COLUMN_SEP)[0]   /* Column seperater set char */

#define IS_START_BLANK_CHAR(c) \
			((c) == ' ' \
			 || (c) == '\t')

#define IS_END_BLANK_CHAR(c) \
			((c) == ' ' \
			 || (c) == '\t' \
			 || (c) == '\r' \
			 || (c) == '\n' \
			 || (c) == COLUMN_SEP_CHAR)

#define IS_BLANK_LINE(c) \
			((c) == '\r' \
			 || (c) == '\n' \
			 || (c) == COMMENT_CHAR)

int read_config(char *config);

#endif /* __CONFIG2_H__ */
