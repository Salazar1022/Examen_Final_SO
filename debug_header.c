#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#define GSEA_MAGIC 0x47534541u

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint8_t  flags;
    uint8_t  comp_alg;
    uint8_t  enc_alg;
    uint8_t  reserved;
    uint64_t orig_size;
    uint32_t crc32;
} gsea_hdr;

int main() {
    int fd = open("tests_auto/test_rle_vig.gsea", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    gsea_hdr h;
    ssize_t r = read(fd, &h, sizeof(h));
    close(fd);
    
    if (r != sizeof(h)) {
        printf("Read error: got %ld bytes, expected %lu\n", (long)r, (unsigned long)sizeof(h));
        return 1;
    }
    
    printf("Header contents:\n");
    printf("  magic:     0x%08X (expected 0x%08X) %s\n", h.magic, GSEA_MAGIC, 
           h.magic == GSEA_MAGIC ? "OK" : "FAIL");
    printf("  version:   %u\n", h.version);
    printf("  flags:     0x%02X (compressed=%d, encrypted=%d)\n", 
           h.flags, h.flags & 1, (h.flags >> 1) & 1);
    printf("  comp_alg:  %u\n", h.comp_alg);
    printf("  enc_alg:   %u\n", h.enc_alg);
    printf("  reserved:  %u\n", h.reserved);
    printf("  orig_size: %lu\n", (unsigned long)h.orig_size);
    printf("  crc32:     0x%08X\n", h.crc32);
    
    return 0;
}
