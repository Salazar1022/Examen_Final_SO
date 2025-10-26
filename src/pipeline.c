#include "pipeline.h"
#include "util.h"
#include "header.h"
#include "rle.h"
#include "lzw.h"
#include "vigenere.h"
#include "feistel.h"
#include "crc32.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

/* Define O_BINARY for Windows compatibility, no-op on Unix */
#ifndef O_BINARY
#define O_BINARY 0
#endif
static int slurp(const char *p, uint8_t **buf, size_t *len){
    int fd = open(p, O_RDONLY | O_BINARY);
    if(fd<0) return -1;
    struct stat st; if(fstat(fd,&st)<0){ close(fd); return -1; }
    size_t n = (size_t)st.st_size;
    uint8_t *b = xmalloc(n);
    size_t off=0;
    while(off<n){
        ssize_t r=read(fd,b+off,n-off);
        if(r<0){ if(errno==EINTR) continue; free(b); close(fd); return -1; }
        if(r==0) break;
        off += (size_t)r;
    }
    close(fd);
    *buf=b; *len=off; return 0;
}
static int spit(const char *p, const uint8_t *buf, size_t len){
    int fd = open(p, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, 0644);
    if(fd<0) return -1;
    int rc = write_all(fd, buf, len);
    close(fd);
    return rc;
}

int run_pipeline_file(const gsea_args *a, const char *in_path, const char *out_path){
    /* Caso 1: Compresión y/o Encriptación (archivo normal -> archivo .gsea) */
    if(a->do_compress || a->do_encrypt){
        uint8_t *buf=NULL; size_t len=0;
        if(slurp(in_path, &buf, &len)<0) return -1;

        uint64_t orig_size = len;
        uint32_t orig_crc  = crc32_of_buffer(buf, len);

        uint8_t *tmp=NULL; size_t tmplen=0;

        /* Paso 1: Comprimir si se solicita */
        if(a->do_compress){
            if(a->comp_alg==ALG_COMP_RLE){
                if(rle_compress(buf, len, &tmp, &tmplen)<0){
                    free(buf); return -1;
                }
                free(buf); buf=tmp; len=tmplen;
            } else if(a->comp_alg==ALG_COMP_LZW){
                if(lzw_compress(buf, len, &tmp, &tmplen)<0){
                    free(buf); return -1;
                }
                free(buf); buf=tmp; len=tmplen;
            } else {
                free(buf); return -1;
            }
        }

        /* Paso 2: Encriptar si se solicita (in-place) */
        if(a->do_encrypt){
            if(a->enc_alg==ALG_ENC_VIG){
                vigenere_xor_apply(buf, len, (const uint8_t*)a->key, a->key_len);
            } else if(a->enc_alg==ALG_ENC_FEISTEL){
                feistel_encrypt(buf, len, (const uint8_t*)a->key, a->key_len);
            } else {
                free(buf); return -1;
            }
        }

        /* Empaquetar con cabecera */
        int fd = open(out_path, O_CREAT|O_TRUNC|O_WRONLY|O_BINARY, 0644);
        if(fd<0){ free(buf); return -1; }
        
        gsea_hdr h = {
            .magic=GSEA_MAGIC,
            .version=GSEA_VERSION,
            .flags=(a->do_compress?1:0) | (a->do_encrypt?2:0),
            .comp_alg=(uint8_t)a->comp_alg,
            .enc_alg=(uint8_t)a->enc_alg,
            .reserved=0,
            .orig_size=orig_size,
            .crc32=orig_crc
        };
        
        if(gsea_write_header(fd,&h)<0 || write_all(fd, buf, len)<0){
            close(fd); free(buf); return -1;
        }
        close(fd);
        free(buf);
        return 0;
    }

    /* Caso 2: Descompresión y/o Desencriptación (archivo .gsea -> archivo normal) */
    if(a->do_decompress || a->do_decrypt){
        int fd = open(in_path, O_RDONLY | O_BINARY);
        if(fd<0) return -1;

        /* Leer cabecera */
        gsea_hdr h;
        if(gsea_read_header(fd, &h)<0){
            close(fd); return -1;
        }

        /* Validar magic */
        if(h.magic != GSEA_MAGIC){
            close(fd); return -1;
        }

        /* Leer datos encriptados/comprimidos */
        struct stat st; 
        if(fstat(fd,&st)<0){ close(fd); return -1; }
        size_t data_size = (size_t)st.st_size - sizeof(gsea_hdr);
        uint8_t *buf = xmalloc(data_size);
        
        size_t off=0;
        while(off<data_size){
            ssize_t r=read(fd,buf+off,data_size-off);
            if(r<0){ 
                if(errno==EINTR) continue; 
                free(buf); close(fd); return -1; 
            }
            if(r==0) break;
            off += (size_t)r;
        }
        close(fd);
        size_t len = off;

        /* Paso 1: Desencriptar si está encriptado */
        if(h.flags & 2){  /* bit1 = encrypted */
            if(h.enc_alg==ALG_ENC_VIG){
                vigenere_xor_apply(buf, len, (const uint8_t*)a->key, a->key_len);
            } else if(h.enc_alg==ALG_ENC_FEISTEL){
                feistel_decrypt(buf, len, (const uint8_t*)a->key, a->key_len);
            } else {
                free(buf); return -1;
            }
        }

        /* Paso 2: Descomprimir si está comprimido */
        if(h.flags & 1){  /* bit0 = compressed */
            uint8_t *tmp=NULL; size_t tmplen=0;
            if(h.comp_alg==ALG_COMP_RLE){
                if(rle_decompress(buf, len, &tmp, &tmplen)<0){
                    free(buf); return -1;
                }
                free(buf); buf=tmp; len=tmplen;
            } else if(h.comp_alg==ALG_COMP_LZW){
                if(lzw_decompress(buf, len, &tmp, &tmplen)<0){
                    free(buf); return -1;
                }
                free(buf); buf=tmp; len=tmplen;
            } else {
                free(buf); return -1;
            }
        }

        /* Verificar CRC32 */
        uint32_t computed_crc = crc32_of_buffer(buf, len);
        if(computed_crc != h.crc32){
            free(buf); return -1;  /* Datos corruptos */
        }

        /* Escribir archivo de salida */
        if(spit(out_path, buf, len)<0){
            free(buf); return -1;
        }

        free(buf);
        return 0;
    }

    /* Caso 3: Solo copiar (sin operaciones) */
    uint8_t *buf=NULL; size_t len=0;
    if(slurp(in_path, &buf, &len)<0) return -1;
    int rc = spit(out_path, buf, len);
    free(buf);
    return rc;
}
