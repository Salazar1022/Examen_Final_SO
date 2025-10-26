#pragma once
#include <stdint.h>
#include <stddef.h>

/**
 * Cifrado Feistel - Red de Feistel simplificada
 * Implementación de 16 rondas con bloques de 64 bits
 * Basado en la estructura usada en DES pero simplificado
 */

void feistel_encrypt(uint8_t *data, size_t len, const uint8_t *key, size_t key_len);
void feistel_decrypt(uint8_t *data, size_t len, const uint8_t *key, size_t key_len);
