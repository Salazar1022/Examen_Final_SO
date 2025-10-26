#include "lzw.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

#define LZW_MAX_CODE 4096  /* 12 bits */
#define LZW_INIT_DICT_SIZE 256

/* Estructura simplificada para compresión */
typedef struct dict_entry {
    int parent;
    uint8_t byte;
} dict_entry;

int lzw_compress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen) {
    if (!in || inlen == 0) {
        *out = NULL;
        *outlen = 0;
        return 0;
    }
    
    /* Diccionario hash simple - usaremos array directo */
    dict_entry dict[LZW_MAX_CODE];
    
    /* Inicializar con caracteres ASCII */
    for (int i = 0; i < 256; i++) {
        dict[i].parent = -1;
        dict[i].byte = (uint8_t)i;
    }
    
    int next_code = LZW_INIT_DICT_SIZE;
    size_t cap = inlen + 1024;  /* Estimación inicial */
    uint8_t *buffer = (uint8_t*)malloc(cap);
    if (!buffer) return -1;
    size_t pos = 0;
    
    int current_code = in[0];  /* Primer byte */
    
    for (size_t i = 1; i < inlen; i++) {
        uint8_t byte = in[i];
        
        /* Buscar si existe current_code + byte en diccionario */
        int found = -1;
        for (int j = LZW_INIT_DICT_SIZE; j < next_code; j++) {
            if (dict[j].parent == current_code && dict[j].byte == byte) {
                found = j;
                break;
            }
        }
        
        if (found >= 0) {
            /* Secuencia existe */
            current_code = found;
        } else {
            /* Secuencia no existe - emitir código actual */
            /* Escribir como 2 bytes */
            if (pos + 2 > cap) {
                cap *= 2;
                buffer = realloc(buffer, cap);
            }
            buffer[pos++] = (current_code >> 8) & 0xFF;
            buffer[pos++] = current_code & 0xFF;
            
            /* Agregar nueva entrada si hay espacio */
            if (next_code < LZW_MAX_CODE) {
                dict[next_code].parent = current_code;
                dict[next_code].byte = byte;
                next_code++;
            }
            
            /* Reiniciar con el byte actual */
            current_code = byte;
        }
    }
    
    /* Emitir último código */
    if (pos + 2 > cap) {
        cap += 2;
        buffer = realloc(buffer, cap);
    }
    buffer[pos++] = (current_code >> 8) & 0xFF;
    buffer[pos++] = current_code & 0xFF;
    
    *out = buffer;
    *outlen = pos;
    return 0;
}

int lzw_decompress(const uint8_t *in, size_t inlen, uint8_t **out, size_t *outlen) {
    if (!in || inlen == 0 || inlen % 2 != 0) {
        *out = NULL;
        *outlen = 0;
        return -1;
    }
    
    /* Tabla de descompresión */
    uint8_t *table[LZW_MAX_CODE];
    size_t table_len[LZW_MAX_CODE];
    
    /* Inicializar con caracteres ASCII */
    for (int i = 0; i < 256; i++) {
        table[i] = (uint8_t*)malloc(1);
        if (!table[i]) {
            for (int j = 0; j < i; j++) free(table[j]);
            return -1;
        }
        table[i][0] = (uint8_t)i;
        table_len[i] = 1;
    }
    
    for (int i = 256; i < LZW_MAX_CODE; i++) {
        table[i] = NULL;
        table_len[i] = 0;
    }
    
    int next_code = LZW_INIT_DICT_SIZE;
    size_t out_cap = inlen * 2 + 1024;
    uint8_t *output = (uint8_t*)malloc(out_cap);
    if (!output) {
        for (int i = 0; i < 256; i++) free(table[i]);
        return -1;
    }
    size_t out_pos = 0;
    
    /* Leer primer código */
    int prev_code = (in[0] << 8) | in[1];
    if (prev_code < 0 || prev_code >= LZW_INIT_DICT_SIZE) {
        free(output);
        for (int i = 0; i < LZW_MAX_CODE; i++) {
            if (table[i]) free(table[i]);
        }
        return -1;
    }
    
    /* Emitir primer código */
    if (out_pos + table_len[prev_code] > out_cap) {
        out_cap *= 2;
        output = realloc(output, out_cap);
    }
    memcpy(output + out_pos, table[prev_code], table_len[prev_code]);
    out_pos += table_len[prev_code];
    
    /* Procesar códigos restantes */
    for (size_t i = 2; i < inlen; i += 2) {
        int code = (in[i] << 8) | in[i + 1];
        
        uint8_t *entry_data;
        size_t entry_len;
        
        if (code < next_code && table[code]) {
            /* Código existe en la tabla */
            entry_data = table[code];
            entry_len = table_len[code];
        } else if (code == next_code) {
            /* Caso especial: código = next_code */
            entry_len = table_len[prev_code] + 1;
            entry_data = (uint8_t*)malloc(entry_len);
            if (!entry_data) break;
            memcpy(entry_data, table[prev_code], table_len[prev_code]);
            entry_data[entry_len - 1] = table[prev_code][0];
            
            /* Agregar a tabla */
            if (next_code < LZW_MAX_CODE) {
                table[next_code] = entry_data;
                table_len[next_code] = entry_len;
                next_code++;
            } else {
                free(entry_data);
                break;
            }
        } else {
            /* Código inválido */
            break;
        }
        
        /* Emitir entrada */
        if (out_pos + entry_len > out_cap) {
            out_cap = out_pos + entry_len + 1024;
            output = realloc(output, out_cap);
        }
        memcpy(output + out_pos, entry_data, entry_len);
        out_pos += entry_len;
        
        /* Agregar nueva entrada a la tabla */
        if (code != next_code && next_code < LZW_MAX_CODE) {
            table[next_code] = (uint8_t*)malloc(table_len[prev_code] + 1);
            if (table[next_code]) {
                memcpy(table[next_code], table[prev_code], table_len[prev_code]);
                table[next_code][table_len[prev_code]] = entry_data[0];
                table_len[next_code] = table_len[prev_code] + 1;
                next_code++;
            }
        }
        
        prev_code = code;
    }
    
    /* Liberar tabla */
    for (int i = 0; i < LZW_MAX_CODE; i++) {
        if (table[i]) free(table[i]);
    }
    
    *out = output;
    *outlen = out_pos;
    return 0;
}
