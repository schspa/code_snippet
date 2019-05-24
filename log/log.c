#include <stdio.h>

#define __DEBUG__

#ifdef __DEBUG__
#define dprintf(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#define dperror(str)      perror(str)
#define dlog(fmt, ...)    fprintf(stderr, "[%s:%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#define dperror(str)
#define dlog(fmt, ...)
#endif

int main(int argc, char *argv[])
{
	dprintf("%s %d\n", "hello", 1);
	dperror("hello 2");
	dlog("%s %d\n", "hello", 3);
	return 0;
}
