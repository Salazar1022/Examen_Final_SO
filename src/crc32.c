#include "crc32.h"
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
uint32_t crc32_update(uint32_t crc, const uint8_t *buf, size_t len){
    if(!initd) init_tab();
    crc = ~crc;
    for(size_t i=0;i<len;i++) crc = T[(crc^buf[i])&0xFF] ^ (crc>>8);
    return ~crc;
}
uint32_t crc32_of_buffer(const uint8_t *buf, size_t len){
    return crc32_update(0, buf, len);
}
