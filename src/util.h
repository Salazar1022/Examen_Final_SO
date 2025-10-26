#pragma once
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define GSEA_BUFSZ (1<<16)  /* 64 KiB */

ssize_t read_all(int fd, uint8_t *buf, size_t cap);
int     write_all(int fd, const uint8_t *buf, size_t len);
void*   xmalloc(size_t n);
