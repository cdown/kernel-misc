#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <linux/fs.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    int generation;
    int fd;

    if (argc != 2)
        return EXIT_FAILURE;

    fd = open(argv[1], O_RDONLY);

    if (fd < 0)
        return EXIT_FAILURE;

    if (ioctl(fd, FS_IOC_GETVERSION, &generation)) {
        perror("ioctl");
        return EXIT_FAILURE;
    }

    printf("%d\n", generation);
}
