#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

static uint32_t T[256];
static int initd=0;

static void init_tab(void){
    for (uint32_t i=0;i<256;i++){
        uint32_t c=i;
        for(int j=0;j<8;j++) c = (c&1)?(0xEDB88320u^(c>>1)):(c>>1);
        T[i]=c;
    }
    initd=1;
}

uint32_t crc32_of_buffer(const uint8_t *buf, size_t len){
    if(!initd) init_tab();
    uint32_t crc = ~0;
    for(size_t i=0;i<len;i++) crc = T[(crc^buf[i])&0xFF] ^ (crc>>8);
    return ~crc;
}

int main() {
    int fd = open("tests_auto/test_enc.txt", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    struct stat st;
    fstat(fd, &st);
    size_t size = st.st_size;
    
    uint8_t *buf = malloc(size);
    read(fd, buf, size);
    close(fd);
    
    uint32_t crc = crc32_of_buffer(buf, size);
    
    printf("File: test_enc.txt\n");
    printf("Size: %lu bytes\n", (unsigned long)size);
    printf("CRC32: 0x%08X\n", crc);
    
    free(buf);
    return 0;
}
