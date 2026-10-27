#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "file_ops.h"

static void usage(const char *program) {
    printf("Використання: %s [опції] файл\n"
           "  -h, --help             показати довідку\n"
           "  -o, --output ФАЙЛ     записати результат у файл\n"
           "  -p, --position ЧИСЛО  початкова позиція в байтах\n"
           "  -c, --count ЧИСЛО     кількість байтів для читання\n"
           "  -n, --newline         додати символ нового рядка\n", program);
}

int main(int argc, char **argv) {
    static const struct option opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"output", required_argument, NULL, 'o'},
        {"position", required_argument, NULL, 'p'},
        {"count", required_argument, NULL, 'c'},
        {"newline", no_argument, NULL, 'n'},
        {NULL, 0, NULL, 0}
    };
    const char *output = NULL;
    uintmax_t position = 0;
    uintmax_t remaining = 0;
    int limited = 0;
    int newline = 0;
    int opt;

    while ((opt = getopt_long(argc, argv, "ho:p:c:n", opts, NULL)) != -1) {
        switch (opt) {
            case 'h': usage(argv[0]); return 0;
            case 'o': output = optarg; break;
            case 'p':
                if (parse_number(optarg, &position) != 0 || position > INT64_MAX) {
                    fprintf(stderr, "Помилка: неправильна позиція у файлі.\n");
                    return 2;
                }
                break;
            case 'c':
                if (parse_number(optarg, &remaining) != 0) {
                    fprintf(stderr, "Помилка: неправильна кількість байтів.\n");
                    return 2;
                }
                limited = 1;
                break;
            case 'n': newline = 1; break;
            default: usage(argv[0]); return 2;
        }
    }
    if (argc - optind != 1) {
        fprintf(stderr, "Помилка: потрібно вказати один вхідний файл.\n");
        usage(argv[0]);
        return 2;
    }
    const char *input = argv[optind];
    int source = open(input, O_RDONLY);
    if (source < 0) {
        perror("Не вдалося відкрити вхідний файл");
        return 1;
    }
    if (lseek(source, (off_t)position, SEEK_SET) == (off_t)-1) {
        perror("Не вдалося змінити позицію читання");
        close(source);
        return 1;
    }
    int target = STDOUT_FILENO;
    if (output) {
        struct stat a, b;
        if (fstat(source, &a) != 0) {
            perror("Не вдалося отримати відомості про вхідний файл");
            close(source);
            return 1;
        }
        if (stat(output, &b) == 0 && a.st_dev == b.st_dev && a.st_ino == b.st_ino) {
            fprintf(stderr, "Помилка: вхідний і вихідний файли збігаються.\n");
            close(source);
            return 2;
        }
        target = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (target < 0) {
            perror("Не вдалося відкрити вихідний файл");
            close(source);
            return 1;
        }
    }
    int result = 0;
    unsigned char buffer[8192];
    while (!limited || remaining > 0) {
        size_t request = sizeof(buffer);
        if (limited && remaining < request) request = (size_t)remaining;
        ssize_t got = read(source, buffer, request);
        if (got < 0 && errno == EINTR) continue;
        if (got < 0) {
            perror("Помилка читання");
            result = 1;
            break;
        }
        if (got == 0) break;
        if (write_all(target, buffer, (size_t)got) != 0) {
            perror("Помилка запису");
            result = 1;
            break;
        }
        if (limited) remaining -= (uintmax_t)got;
    }
    if (!result && newline && write_all(target, (const unsigned char *)"\n", 1) != 0) {
        perror("Помилка запису нового рядка");
        result = 1;
    }
    if (output && close(target) != 0) {
        perror("Помилка закриття вихідного файлу");
        result = 1;
    }
    if (close(source) != 0) {
        perror("Помилка закриття вхідного файлу");
        result = 1;
    }
    return result;
}
