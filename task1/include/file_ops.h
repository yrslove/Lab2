#ifndef FILE_OPS_H
#define FILE_OPS_H
#include <inttypes.h>
#include <stddef.h>
int parse_number(const char *value, uintmax_t *number);
int write_all(int fd, const unsigned char *data, size_t size);
#endif
