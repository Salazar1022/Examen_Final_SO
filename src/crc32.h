#pragma once
#include <stddef.h>
#include <stdint.h>
uint32_t crc32_update(uint32_t crc, const uint8_t *buf, size_t len);
uint32_t crc32_of_buffer(const uint8_t *buf, size_t len);
