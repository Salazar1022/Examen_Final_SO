#pragma once
#include <stddef.h>
#include <stdint.h>

/* RLE byte-oriented: [count(1..255), value] secuencias */
int rle_compress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen);
int rle_decompress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen);
