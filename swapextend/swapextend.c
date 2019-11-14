/* Call new swapextend syscall. Designed for new online extension patchset. */

#define _GNU_SOURCE
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int ret;
	int SYS_swapextend;
	const char *filename = NULL;
	long desired_size;

	if (argc != 4) {
		fprintf(stderr, "usage: %s filename size\n", argv[0]);
		return 1;
	}

	SYS_swapextend = atoi(argv[1]);
	filename = argv[2];
	desired_size = strtol(argv[3], NULL, 10);

	ret = syscall(SYS_swapextend, filename, desired_size);
	printf("%d\n", ret);
	printf("%s\n", strerror(errno));

	return !!ret;
}
