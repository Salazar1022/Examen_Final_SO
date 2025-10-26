#pragma once
#include <stddef.h>
#include <stdint.h>

void vigenere_xor_apply(uint8_t *buf, size_t len, const uint8_t *key, size_t klen);
