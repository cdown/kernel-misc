/*
 * Leak memory, showing how long it takes to do so for each megabyte.
 *
 * I created this as part of testing my memory.high allocator delay patchset,
 * which became part of the kernel in 5.4.
 *
 * You can see how to use it in 0e4b01df (here called mdf):
 * https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/patch/?id=0e4b01df865935007bd712cbc8e7299005b28894
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>

#define MIB (1UL << 20)
#define MMAP_BYTES 4 * MIB

uintmax_t get_usec(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (uintmax_t)t.tv_sec * 1000000 + t.tv_usec;
}

int main(void) {
    uintmax_t last_time = get_usec(), cur_bytes = 0;
    unsigned int last_mb = 0;

    while (1) {
        char* buf;
        unsigned int cur_mb;
        uintmax_t this_time;

        buf = mmap(NULL, MMAP_BYTES, PROT_READ | PROT_WRITE,
                   MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if (!buf) return EXIT_FAILURE;

        memset(buf, 0, MMAP_BYTES);
        (void)buf;

        cur_bytes += MMAP_BYTES;
        cur_mb = cur_bytes / MIB;

        if (cur_mb == last_mb) continue;

        /* we're on to a new mb */
        this_time = get_usec();
        printf("%d %ju\n", cur_mb, (this_time - last_time));
        last_time = this_time;
        last_mb = cur_mb;

        /* buf is leaked */
    }

    return EXIT_SUCCESS;
}
