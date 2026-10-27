#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "file_info.h"

static char file_type(mode_t mode) {
    if (S_ISDIR(mode)) return 'd';
    if (S_ISLNK(mode)) return 'l';
    if (S_ISCHR(mode)) return 'c';
    if (S_ISBLK(mode)) return 'b';
    if (S_ISFIFO(mode)) return 'p';
    if (S_ISSOCK(mode)) return 's';
    return '-';
}

void permissions(mode_t mode, char text[11]) {
    const mode_t bits[9] = {
        S_IRUSR, S_IWUSR, S_IXUSR,
        S_IRGRP, S_IWGRP, S_IXGRP,
        S_IROTH, S_IWOTH, S_IXOTH
    };
    const char symbols[3] = {'r', 'w', 'x'};
    text[0] = file_type(mode);
    for (int i = 0; i < 9; ++i) text[i + 1] = (mode & bits[i]) ? symbols[i % 3] : '-';
    if (mode & S_ISUID) text[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) text[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) text[9] = (mode & S_IXOTH) ? 't' : 'T';
    text[10] = '\0';
}

