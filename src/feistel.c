#include "feistel.h"
#include <string.h>

#define FEISTEL_ROUNDS 16
#define BLOCK_SIZE 8  /* 64 bits */

/**
 * Función de hash simple para generar subclaves
 */
static uint32_t hash_subkey(const uint8_t *key, size_t key_len, int round) {
    uint32_t h = 0x9e3779b9 + round;  /* Constante golden ratio */
    for (size_t i = 0; i < key_len; i++) {
        h ^= key[i];
        h *= 0x01000193;  /* FNV prime */
        h ^= (h >> 16);
    }
    return h;
}

/**
 * Función F de Feistel - mezcla half-block con subkey
 * Implementa sustitución y permutación simple
 */
static uint32_t feistel_f(uint32_t half, uint32_t subkey) {
    /* XOR con subkey */
    uint32_t x = half ^ subkey;
    
    /* S-box simplificada: sustitución por bytes */
    uint8_t *bytes = (uint8_t*)&x;
    for (int i = 0; i < 4; i++) {
        /* Sustitución no lineal simple */
        uint8_t b = bytes[i];
        b = ((b << 1) | (b >> 7)) ^ 0x63;  /* Rotación + XOR */
        b = ((b << 3) | (b >> 5)) ^ b;
        bytes[i] = b;
    }
    
    /* Permutación: rotar bits */
    x = (x << 7) | (x >> 25);
    
    /* Más difusión */
    x ^= (x >> 16);
    x *= 0x85ebca6b;
    x ^= (x >> 13);
    
    return x;
}

/**
 * Procesa un bloque de 64 bits (8 bytes)
 */
static void feistel_block_encrypt(uint8_t *block, const uint8_t *key, size_t key_len) {
    /* Dividir bloque en dos mitades de 32 bits */
    uint32_t left = ((uint32_t)block[0] << 24) | 
                    ((uint32_t)block[1] << 16) |
                    ((uint32_t)block[2] << 8) | 
                    ((uint32_t)block[3]);
    
    uint32_t right = ((uint32_t)block[4] << 24) | 
                     ((uint32_t)block[5] << 16) |
                     ((uint32_t)block[6] << 8) | 
                     ((uint32_t)block[7]);
    
    /* 16 rondas de Feistel */
    for (int round = 0; round < FEISTEL_ROUNDS; round++) {
        uint32_t subkey = hash_subkey(key, key_len, round);
        uint32_t f_result = feistel_f(right, subkey);
        uint32_t new_right = left ^ f_result;
        left = right;
        right = new_right;
    }
    
    /* Swap final (cancelado en decriptación) */
    uint32_t temp = left;
    left = right;
    right = temp;
    
    /* Escribir resultado */
    block[0] = (left >> 24) & 0xFF;
    block[1] = (left >> 16) & 0xFF;
    block[2] = (left >> 8) & 0xFF;
    block[3] = left & 0xFF;
    
    block[4] = (right >> 24) & 0xFF;
    block[5] = (right >> 16) & 0xFF;
    block[6] = (right >> 8) & 0xFF;
    block[7] = right & 0xFF;
}

static void feistel_block_decrypt(uint8_t *block, const uint8_t *key, size_t key_len) {
    /* Dividir bloque en dos mitades de 32 bits */
    uint32_t left = ((uint32_t)block[0] << 24) | 
                    ((uint32_t)block[1] << 16) |
                    ((uint32_t)block[2] << 8) | 
                    ((uint32_t)block[3]);
    
    uint32_t right = ((uint32_t)block[4] << 24) | 
                     ((uint32_t)block[5] << 16) |
                     ((uint32_t)block[6] << 8) | 
                     ((uint32_t)block[7]);
    
    /* Deshacer swap final de encriptación */
    uint32_t temp = left;
    left = right;
    right = temp;
    
    /* 16 rondas en orden inverso - aplicar F sobre left */
    for (int round = FEISTEL_ROUNDS - 1; round >= 0; round--) {
        uint32_t subkey = hash_subkey(key, key_len, round);
        uint32_t f_result = feistel_f(left, subkey);  /* F sobre LEFT en decrypt */
        uint32_t new_left = right ^ f_result;
        right = left;
        left = new_left;
    }
    
    /* Escribir resultado */
    block[0] = (left >> 24) & 0xFF;
    block[1] = (left >> 16) & 0xFF;
    block[2] = (left >> 8) & 0xFF;
    block[3] = left & 0xFF;
    
    block[4] = (right >> 24) & 0xFF;
    block[5] = (right >> 16) & 0xFF;
    block[6] = (right >> 8) & 0xFF;
    block[7] = right & 0xFF;
}

void feistel_encrypt(uint8_t *data, size_t len, const uint8_t *key, size_t key_len) {
    if (!data || len == 0 || !key || key_len == 0) return;
    
    /* Procesar bloques completos de 8 bytes */
    size_t full_blocks = len / BLOCK_SIZE;
    for (size_t i = 0; i < full_blocks; i++) {
        feistel_block_encrypt(data + i * BLOCK_SIZE, key, key_len);
    }
    
    /* Procesar bytes restantes con padding implícito (XOR simple) */
    size_t remaining = len % BLOCK_SIZE;
    if (remaining > 0) {
        uint8_t pad_block[BLOCK_SIZE] = {0};
        memcpy(pad_block, data + full_blocks * BLOCK_SIZE, remaining);
        
        /* Pad con bytes del tamaño restante */
        for (size_t i = remaining; i < BLOCK_SIZE; i++) {
            pad_block[i] = (uint8_t)remaining;
        }
        
        feistel_block_encrypt(pad_block, key, key_len);
        memcpy(data + full_blocks * BLOCK_SIZE, pad_block, remaining);
    }
}

void feistel_decrypt(uint8_t *data, size_t len, const uint8_t *key, size_t key_len) {
    if (!data || len == 0 || !key || key_len == 0) return;
    
    /* Procesar bloques completos de 8 bytes */
    size_t full_blocks = len / BLOCK_SIZE;
    for (size_t i = 0; i < full_blocks; i++) {
        feistel_block_decrypt(data + i * BLOCK_SIZE, key, key_len);
    }
    
    /* Procesar bytes restantes (mismo padding que encrypt) */
    size_t remaining = len % BLOCK_SIZE;
    if (remaining > 0) {
        uint8_t pad_block[BLOCK_SIZE] = {0};
        memcpy(pad_block, data + full_blocks * BLOCK_SIZE, remaining);
        
        /* Pad con bytes del tamaño restante */
        for (size_t i = remaining; i < BLOCK_SIZE; i++) {
            pad_block[i] = (uint8_t)remaining;
        }
        
        feistel_block_decrypt(pad_block, key, key_len);
        memcpy(data + full_blocks * BLOCK_SIZE, pad_block, remaining);
    }
}
