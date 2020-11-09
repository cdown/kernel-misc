/* Wait for an arbitrary pid to finish without polling. */

#include <errno.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

/* glibc doesn't have a proper wrapper for this yet as of 2.32 */
#define pidfd_open(pid, flags) syscall(__NR_pidfd_open, pid, flags)

int pid_t_from_charbuf(char *input, pid_t *output)
{
	char *buf;
	uintmax_t raw_pid;

	errno = 0;
	raw_pid = strtoumax(input, &buf, 10);

	if (errno > 0 || input == buf || *buf != '\0' ||
	    raw_pid != (uintmax_t)(pid_t)raw_pid)
		return -EINVAL;

	*output = (pid_t)raw_pid;

	return 0;
}

int main(int argc, char *argv[])
{
	int pid_fd, r;
	pid_t pid;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s pid\n", argv[0]);
		return EXIT_FAILURE;
	}

	r = pid_t_from_charbuf(argv[1], &pid);
	if (r < 0) {
		fprintf(stderr, "invalid pid '%s'\n", argv[1]);
		return EXIT_FAILURE;
	}

	pid_fd = pidfd_open(pid, 0);
	if (pid_fd < 0) {
		perror("pidfd_open");
		return EXIT_FAILURE;
	}

	/* POLLIN fires on PID exit */
	r = poll(&(struct pollfd){ .fd = pid_fd, .events = POLLIN }, 1, -1);
	if (r < 0) {
		perror("poll");
		close(pid_fd);
		return EXIT_FAILURE;
	}

	close(pid_fd);
}
