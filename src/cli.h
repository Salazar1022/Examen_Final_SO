#pragma once
#include <stddef.h>
#include <stdint.h>
#include "header.h"

typedef struct {
    int do_compress, do_decompress, do_encrypt, do_decrypt;
    comp_alg_t comp_alg;
    enc_alg_t  enc_alg;
    const char *in_path;
    const char *out_path;
    const char *key; size_t key_len;
    int max_threads; /* límite de concurrencia */
} gsea_args;

int parse_args(int argc, char **argv, gsea_args *out);
