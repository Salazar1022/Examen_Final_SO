#include "cli.h"
#include <string.h>
#include <stdlib.h>

static int streq(const char *a, const char *b){ return strcmp(a,b)==0; }

int parse_args(int argc, char **argv, gsea_args *o){
    memset(o,0,sizeof(*o));
    o->comp_alg = ALG_COMP_RLE;
    o->enc_alg  = ALG_ENC_VIG;
    o->max_threads = 8; /* por defecto */

    for(int i=1;i<argc;i++){
        char *a = argv[i];
        if(a[0]=='-' && a[1] && a[1]!='-'){ /* -c -d -e -u -i -o -k -t o combinaciones como -ce */
            /* Procesar múltiples flags en una sola opción */
            int has_arg = 0;
            for(int j=1; a[j] && !has_arg; j++){
                switch(a[j]){
                    case 'c': o->do_compress=1; break;
                    case 'd': o->do_decompress=1; break;
                    case 'e': o->do_encrypt=1; break;
                    case 'u': o->do_decrypt=1; break;
                    case 'i': 
                        if(++i<argc) o->in_path=argv[i]; 
                        else return -1; 
                        has_arg = 1; /* tiene argumento, salir del loop */
                        break;
                    case 'o': 
                        if(++i<argc) o->out_path=argv[i]; 
                        else return -1; 
                        has_arg = 1;
                        break;
                    case 'k': 
                        if(++i<argc){ 
                            o->key=argv[i]; 
                            o->key_len=strlen(o->key);
                        } else return -1; 
                        has_arg = 1;
                        break;
                    case 't': 
                        if(++i<argc){ 
                            o->max_threads=atoi(argv[i]); 
                        } else return -1; 
                        has_arg = 1;
                        break;
                    default: return -1;
                }
            }
        } else if(streq(a,"--comp-alg")){
            if(++i>=argc) return -1;
            if(streq(argv[i],"rle")) o->comp_alg=ALG_COMP_RLE;
            else if(streq(argv[i],"lzw")) o->comp_alg=ALG_COMP_LZW;
            else return -1;
        } else if(streq(a,"--enc-alg")){
            if(++i>=argc) return -1;
            if(streq(argv[i],"vigenere")) o->enc_alg=ALG_ENC_VIG;
            else if(streq(argv[i],"feistel")) o->enc_alg=ALG_ENC_FEISTEL;
            else return -1;
        } else {
            return -1;
        }
    }
    if(!o->in_path || !o->out_path) return -1;
    return 0;
}
