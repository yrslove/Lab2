#define _POSIX_C_SOURCE 200809L
#include "file_ops.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int parse_number(const char *value, uintmax_t *number) {
    char *end = NULL;
    if (!value || !*value || *value == '-' || *value == '+') return -1;
    errno = 0;
    uintmax_t parsed = strtoumax(value, &end, 10);
    if (errno || end == value || *end != '\0') return -1;
    *number = parsed;
    return 0;
}

int write_all(int fd, const unsigned char *data, size_t size) {
    size_t done = 0;
    while (done < size) {
        ssize_t n = write(fd, data + done, size - done);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        done += (size_t)n;
    }
    return 0;
}

