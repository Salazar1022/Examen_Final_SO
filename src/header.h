#pragma once
#include <stdint.h>

#define GSEA_MAGIC 0x47534541u /* 'GSEA' */
#define GSEA_VERSION 1

typedef enum { OP_NONE=0, OP_COMPRESS=1, OP_DECOMPRESS=2, OP_ENCRYPT=4, OP_DECRYPT=8 } op_t;
typedef enum { ALG_COMP_NONE=0, ALG_COMP_RLE=1, ALG_COMP_LZW=2 } comp_alg_t;
typedef enum { ALG_ENC_NONE=0, ALG_ENC_VIG=1, ALG_ENC_FEISTEL=2 } enc_alg_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint8_t  flags;       // bit0: compressed? bit1: encrypted?
    uint8_t  comp_alg;    // comp algorithm id
    uint8_t  enc_alg;     // enc algorithm id
    uint8_t  reserved;
    uint64_t orig_size;   // tamaño antes de pipeline
    uint32_t crc32;       // del contenido original
} gsea_hdr;

int gsea_write_header(int fd, const gsea_hdr *h);
int gsea_read_header(int fd, gsea_hdr *h);
