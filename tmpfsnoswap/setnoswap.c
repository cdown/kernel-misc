#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>

#define FS_NOSWAP_FL 0x01000000 /* Do not swap file */

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s [+S|-S] <file>\n", prog_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int fd;
    int flags;
    char op;
    const char *path;

    if (argc != 3) {
        print_usage(argv[0]);
    }

    if ((argv[1][0] != '+' && argv[1][0] != '-') || argv[1][1] != 'S' || argv[1][2] != '\0') {
        fprintf(stderr, "Error: Invalid operator '%s'. Use '+S' or '-S'.\n", argv[1]);
        print_usage(argv[0]);
    }

    op = argv[1][0];
    path = argv[2];

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    if (ioctl(fd, FS_IOC_GETFLAGS, &flags) < 0) {
        perror("ioctl(FS_IOC_GETFLAGS) failed");
        close(fd);
        return EXIT_FAILURE;
    }

    if (op == '+') {
        flags |= FS_NOSWAP_FL;
    } else {
        flags &= ~FS_NOSWAP_FL;
    }

    if (ioctl(fd, FS_IOC_SETFLAGS, &flags) < 0) {
        perror("ioctl(FS_IOC_SETFLAGS) failed");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("Successfully updated flags on %s.\n", path);

    close(fd);
    return EXIT_SUCCESS;
}
