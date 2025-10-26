#pragma once
#include <stdint.h>
#include <stddef.h>

/**
 * LZW (Lempel-Ziv-Welch) - Algoritmo de compresión sin pérdida
 * Utiliza un diccionario dinámico que se construye durante la compresión
 */

int lzw_compress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen);
int lzw_decompress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen);
