#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Copiar las funciones de feistel.c
#define FEISTEL_ROUNDS 16
#define BLOCK_SIZE 8

static uint32_t hash_subkey(const uint8_t *key, size_t key_len, int round) {
    uint32_t h = 0x9e3779b9 + round;
    for (size_t i = 0; i < key_len; i++) {
        h ^= key[i];
        h *= 0x01000193;
        h ^= (h >> 16);
    }
    return h;
}

static uint32_t feistel_f(uint32_t half, uint32_t subkey) {
    uint32_t x = half ^ subkey;
    uint8_t *bytes = (uint8_t*)&x;
    for (int i = 0; i < 4; i++) {
        uint8_t b = bytes[i];
        b = ((b << 1) | (b >> 7)) ^ 0x63;
        b = ((b << 3) | (b >> 5)) ^ b;
        bytes[i] = b;
    }
    x = (x << 7) | (x >> 25);
    x ^= (x >> 16);
    x *= 0x85ebca6b;
    x ^= (x >> 13);
    return x;
}

static void feistel_block_encrypt(uint8_t *block, const uint8_t *key, size_t key_len) {
    uint32_t left = ((uint32_t)block[0] << 24) | 
                    ((uint32_t)block[1] << 16) |
                    ((uint32_t)block[2] << 8) | 
                    ((uint32_t)block[3]);
    
    uint32_t right = ((uint32_t)block[4] << 24) | 
                     ((uint32_t)block[5] << 16) |
                     ((uint32_t)block[6] << 8) | 
                     ((uint32_t)block[7]);
    
    for (int round = 0; round < FEISTEL_ROUNDS; round++) {
        uint32_t subkey = hash_subkey(key, key_len, round);
        uint32_t f_result = feistel_f(right, subkey);
        uint32_t new_right = left ^ f_result;
        left = right;
        right = new_right;
    }
    
    uint32_t temp = left;
    left = right;
    right = temp;
    
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
    uint32_t left = ((uint32_t)block[0] << 24) | 
                    ((uint32_t)block[1] << 16) |
                    ((uint32_t)block[2] << 8) | 
                    ((uint32_t)block[3]);
    
    uint32_t right = ((uint32_t)block[4] << 24) | 
                     ((uint32_t)block[5] << 16) |
                     ((uint32_t)block[6] << 8) | 
                     ((uint32_t)block[7]);
    
    uint32_t temp = left;
    left = right;
    right = temp;
    
    for (int round = FEISTEL_ROUNDS - 1; round >= 0; round--) {
        uint32_t subkey = hash_subkey(key, key_len, round);
        uint32_t f_result = feistel_f(right, subkey);
        uint32_t new_right = left ^ f_result;
        left = right;
        right = new_right;
    }
    
    block[0] = (left >> 24) & 0xFF;
    block[1] = (left >> 16) & 0xFF;
    block[2] = (left >> 8) & 0xFF;
    block[3] = left & 0xFF;
    
    block[4] = (right >> 24) & 0xFF;
    block[5] = (right >> 16) & 0xFF;
    block[6] = (right >> 8) & 0xFF;
    block[7] = right & 0xFF;
}

int main() {
    uint8_t data[8] = {'H', 'O', 'L', 'A', 'M', 'U', 'N', 'D'};
    uint8_t encrypted[8];
    uint8_t key[] = "miClave";
    
    memcpy(encrypted, data, 8);
    
    printf("Original: ");
    for (int i = 0; i < 8; i++) printf("%c", data[i]);
    printf("\n");
    
    printf("Hex:      ");
    for (int i = 0; i < 8; i++) printf("%02X ", data[i]);
    printf("\n\n");
    
    // Encrypt
    uint32_t L = ((uint32_t)encrypted[0] << 24) | ((uint32_t)encrypted[1] << 16) |
                 ((uint32_t)encrypted[2] << 8) | ((uint32_t)encrypted[3]);
    uint32_t R = ((uint32_t)encrypted[4] << 24) | ((uint32_t)encrypted[5] << 16) |
                 ((uint32_t)encrypted[6] << 8) | ((uint32_t)encrypted[7]);
    
    printf("Encrypt:\n");
    printf("  L0 = 0x%08X, R0 = 0x%08X\n", L, R);
    
    for (int round = 0; round < FEISTEL_ROUNDS; round++) {
        uint32_t subkey = hash_subkey(key, strlen((char*)key), round);
        uint32_t f_result = feistel_f(R, subkey);
        uint32_t new_R = L ^ f_result;
        L = R;
        R = new_R;
        if (round == 0 || round == FEISTEL_ROUNDS - 1) {
            printf("  Round %2d: L = 0x%08X, R = 0x%08X\n", round + 1, L, R);
        }
    }
    
    // Swap final
    uint32_t temp = L;
    L = R;
    R = temp;
    printf("  After swap: L = 0x%08X, R = 0x%08X\n", L, R);
    
    encrypted[0] = (L >> 24) & 0xFF;
    encrypted[1] = (L >> 16) & 0xFF;
    encrypted[2] = (L >> 8) & 0xFF;
    encrypted[3] = L & 0xFF;
    encrypted[4] = (R >> 24) & 0xFF;
    encrypted[5] = (R >> 16) & 0xFF;
    encrypted[6] = (R >> 8) & 0xFF;
    encrypted[7] = R & 0xFF;
    
    printf("\nEncrypted:");
    for (int i = 0; i < 8; i++) printf("%02X ", encrypted[i]);
    printf("\n\n");
    
    // Decrypt
    L = ((uint32_t)encrypted[0] << 24) | ((uint32_t)encrypted[1] << 16) |
        ((uint32_t)encrypted[2] << 8) | ((uint32_t)encrypted[3]);
    R = ((uint32_t)encrypted[4] << 24) | ((uint32_t)encrypted[5] << 16) |
        ((uint32_t)encrypted[6] << 8) | ((uint32_t)encrypted[7]);
    
    printf("Decrypt:\n");
    printf("  Input: L = 0x%08X, R = 0x%08X\n", L, R);
    
    // Deshacer swap
    temp = L;
    L = R;
    R = temp;
    printf("  After unswap: L = 0x%08X, R = 0x%08X\n", L, R);
    
    for (int round = FEISTEL_ROUNDS - 1; round >= 0; round--) {
        uint32_t subkey = hash_subkey(key, strlen((char*)key), round);
        uint32_t f_result = feistel_f(L, subkey);  // F sobre LEFT
        uint32_t new_L = R ^ f_result;
        R = L;
        L = new_L;
        if (round == FEISTEL_ROUNDS - 1 || round == 0) {
            printf("  Round %2d: L = 0x%08X, R = 0x%08X\n", round, L, R);
        }
    }
    
    printf("  Final: L = 0x%08X, R = 0x%08X\n", L, R);
    
    encrypted[0] = (L >> 24) & 0xFF;
    encrypted[1] = (L >> 16) & 0xFF;
    encrypted[2] = (L >> 8) & 0xFF;
    encrypted[3] = L & 0xFF;
    encrypted[4] = (R >> 24) & 0xFF;
    encrypted[5] = (R >> 16) & 0xFF;
    encrypted[6] = (R >> 8) & 0xFF;
    encrypted[7] = R & 0xFF;
    
    printf("\nDecrypted: ");
    for (int i = 0; i < 8; i++) printf("%c", encrypted[i]);
    printf("\n");
    
    printf("Hex:      ");
    for (int i = 0; i < 8; i++) printf("%02X ", encrypted[i]);
    printf("\n");
    
    return 0;
}
