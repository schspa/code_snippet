/*
 * ifhwaddr hook - a hook SIOCGIFHWADDR ioctl to give a fake MAC address for apps
 * LD_PRELOAD="hook.so" command
 *
 */

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *) -1l)
#endif

#define REAL_LIBC RTLD_NEXT

__attribute__((constructor)) static void thermal_init(void)
{
}

int ioctl (int __fd, unsigned long int __request, ...) {
    static int (*func_ioctl) (int, unsigned long int, void *) = NULL;
    va_list args;
    void *argp;
    int retval;

    if (! func_ioctl) {
        func_ioctl = (int (*) (int, unsigned long int, void *)) dlsym (REAL_LIBC, "ioctl");
    }
    va_start (args, __request);
    argp = va_arg (args, void *);
    va_end (args);
    struct ifreq *ifr = argp;

    retval = func_ioctl (__fd, __request, argp);

    if (SIOCGIFHWADDR == __request && retval == 0) {

        unsigned char* mac=(unsigned char*)ifr->ifr_hwaddr.sa_data;
        // set to b3:26:83:2c:73:ec
        mac[0] = 0xb3;
        mac[1] = 0x26;
        mac[2] = 0x83;
        mac[3] = 0x2c;
        mac[4] = 0x73;
        mac[5] = 0xec;
    }

    return retval;
}
#if 0
int __xstat(const char *pathname, struct stat *statbuf) {
	static int (*func__xstat) (const char *pathname, struct stat *statbuf) = NULL;
	int retval;

	if (!func__xstat) {
		func__xstat = (int (*) (int, unsigned long int, void *)) dlsym (REAL_LIBC, "__xstat");
	}

	retval = func__xstat(pathname, statbuf);
	printf("__xstat for %s return %d\n", pathname, retval);

	return retval;
}
int __lxstat (int vers, const char *name, struct stat *buf) {
	static int (*func__lxstat)(int vers, const char *name, struct stat *buf) = NULL;
	int retval;

	if (!func__lxstat) {
		func__lxstat = (void *) dlsym(REAL_LIBC, "__lxstat");
	}
	retval = func__lxstat(vers, name, buf);
	printf("__lxstat for %s return %d\n", name, retval);

	return retval;
}


int fstat(int fd, struct stat *buf) {
	static int (*func_fstat)(int fd, struct stat *buf) = NULL;
	int retval;

	if (!func_fstat) {
		func_fstat = (void *) dlsym(REAL_LIBC, "fstat");
	}

	retval = func_fstat(fd, buf);
	printf("fstat for %d return %d\n", fd, retval);

	return retval;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	static int (*func_gettimeofday)(struct timeval *tv, struct timezone *tz) = NULL;
	int retval;

	if (!func_gettimeofday) {
		func_gettimeofday = (int (*) (int, unsigned long int, void *)) dlsym (REAL_LIBC, "gettimeofday");
	}

	retval = func_gettimeofday(tv, tz);
	if (!retval) {
		printf("gettimeofday return %lu\n", tv->tv_sec);
		tv->tv_sec -= 60 * 60 * 24 * 90;
		printf("gettimeofday change to %lu\n", tv->tv_sec);
	}

	return retval;
}

#endif
