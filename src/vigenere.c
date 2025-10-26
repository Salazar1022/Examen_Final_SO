#include "vigenere.h"

void vigenere_xor_apply(uint8_t *buf, size_t len, const uint8_t *key, size_t klen){
    if(klen==0) return;
    for(size_t i=0;i<len;i++){
        buf[i] ^= key[i % klen];
    }
}
