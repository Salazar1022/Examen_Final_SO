#include "rle.h"
#include "util.h"

int rle_compress(const uint8_t *in, size_t n, uint8_t **out, size_t *outlen){
    /* peor caso ~2*n */
    uint8_t *o = xmalloc(n*2 + 2);
    size_t w = 0;
    for(size_t i=0;i<n;){
        uint8_t v=in[i];
        size_t j=i+1, cnt=1;
        while(j<n && in[j]==v && cnt<255){ j++; cnt++; }
        o[w++] = (uint8_t)cnt;
        o[w++] = v;
        i = j;
    }
    *out=o; *outlen=w; return 0;
}
int rle_decompress(const uint8_t *in, size_t n, uint8_t **out, size_t *outlen){
    /* estimación: necesitamos 255x; hacemos dos pasadas: */
    size_t cap = 0;
    for(size_t i=0;i+1<n;i+=2) cap += in[i];
    uint8_t *o = xmalloc(cap);
    size_t w=0;
    for(size_t i=0;i+1<n;i+=2){
        uint8_t cnt=in[i], val=in[i+1];
        for(uint8_t k=0;k<cnt;k++) o[w++]=val;
    }
    *out=o; *outlen=w; return 0;
}
