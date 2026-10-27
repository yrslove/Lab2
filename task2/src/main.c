#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "file_info.h"

static void usage(const char *program) {
    printf("Використання: %s [опції] [каталог]\n"
           "  -h, --help  показати довідку\n"
           "  -l, --long  докладні відомості про об'єкти\n"
           "  -a, --all   показати також приховані об'єкти\n", program);
}

int main(int argc, char **argv) {
    static const struct option opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"long", no_argument, NULL, 'l'},
        {"all", no_argument, NULL, 'a'},
        {NULL, 0, NULL, 0}
    };
    int show_all = 0;
    int long_format = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "hla", opts, NULL)) != -1) {
        switch (opt) {
            case 'h': usage(argv[0]); return 0;
            case 'l': long_format = 1; break;
            case 'a': show_all = 1; break;
            default: usage(argv[0]); return 2;
        }
    }
    if (argc - optind > 1) {
        fprintf(stderr, "Помилка: допускається не більше одного каталогу.\n");
        return 2;
    }
    const char *path = optind < argc ? argv[optind] : ".";
    DIR *directory = opendir(path);
    if (!directory) {
        perror("Не вдалося відкрити каталог");
        return 1;
    }
    struct dirent **entries = NULL;
    int count = scandir(path, &entries, NULL, alphasort);
    if (count < 0) {
        perror("Помилка читання каталогу");
        closedir(directory);
        return 1;
    }
    int status = 0;
    for (int i = 0; i < count; ++i) {
        const char *name = entries[i]->d_name;
        if (!show_all && name[0] == '.') {
            free(entries[i]);
            continue;
        }
        if (long_format) {
            struct stat st;
            if (fstatat(dirfd(directory), name, &st, AT_SYMLINK_NOFOLLOW) != 0) {
                fprintf(stderr, "Не вдалося прочитати відомості про %s: %s\n", name, strerror(errno));
                status = 1;
                free(entries[i]);
                continue;
            }
            char mode[11];
            char modified[32];
            permissions(st.st_mode, mode);
            struct tm tm_value;
            if (localtime_r(&st.st_mtime, &tm_value) && strftime(modified, sizeof(modified), "%d.%m.%Y %H:%M", &tm_value)) {
                printf("%s %10jd  %s  %s\n", mode, (intmax_t)st.st_size, modified, name);
            } else {
                printf("%s %10jd  %s\n", mode, (intmax_t)st.st_size, name);
            }
        } else {
            printf("%s\n", name);
        }
        free(entries[i]);
    }
    free(entries);
    if (closedir(directory) != 0) {
        perror("Помилка закриття каталогу");
        status = 1;
    }
    return status;
}
