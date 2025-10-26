#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

ssize_t read_all(int fd, uint8_t *buf, size_t cap){
    size_t off=0; 
    while(off<cap){
        ssize_t r=read(fd, buf+off, cap-off);
        if(r<0){ if(errno==EINTR) continue; return -1; }
        if(r==0) break;
        off += (size_t)r;
        if(r==0) break;
        if(off>0) break; /* leemos al menos un bloque si existe */
    }
    return (ssize_t)off;
}
int write_all(int fd, const uint8_t *buf, size_t len){
    size_t off=0;
    while(off<len){
        ssize_t w=write(fd, buf+off, len-off);
        if(w<0){ if(errno==EINTR) continue; return -1; }
        off += (size_t)w;
    }
    return 0;
}
void* xmalloc(size_t n){
    void *p = malloc(n);
    if(!p) _exit(12);
    return p;
}
