#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h> 
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	struct timeval t1, t2;

	if (gettimeofday(&t1, NULL)) {
		perror("gettimeofday");
		exit(1);
	}
	if (gettimeofday(&t2, NULL)) {
		perror("gettimeofday");
		exit(1);
	}
	printf("time0: %lu  time1: %lu  diff: %lu\n", (unsigned long)t1.tv_usec,
	       (unsigned long)t2.tv_usec, (unsigned long)((1000000+t2.tv_usec-t1.tv_usec) % 1000000));
	exit(0);
}
