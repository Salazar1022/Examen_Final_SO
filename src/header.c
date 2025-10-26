#include "header.h"
#include <unistd.h>
#include <errno.h>

int gsea_write_header(int fd, const gsea_hdr *h){
    ssize_t w = write(fd, h, sizeof(*h));
    return (w == (ssize_t)sizeof(*h)) ? 0 : -1;
}
int gsea_read_header(int fd, gsea_hdr *h){
    ssize_t r = read(fd, h, sizeof(*h));
    return (r == (ssize_t)sizeof(*h)) ? 0 : -1;
}
