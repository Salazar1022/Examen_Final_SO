#include "cli.h"
#include "worker.h"
#include "util.h"
#include <unistd.h>   // write
#include <string.h>

static void usage(void){
    const char *u =
"Uso: ./gsea [ops] --comp-alg {rle|lzw} --enc-alg {vigenere|feistel} -i IN -o OUT [-k KEY] [-t MAX_THREADS]\n"
"Ops: -c (comprimir) -d (descomprimir) -e (encriptar) -u (desencriptar)\n"
"Ej:  ./gsea -ce --comp-alg rle --enc-alg vigenere -i ./in/ -o ./out/ -k secreto\n";
    write(1,u, (unsigned)strlen(u));
}

int main(int argc, char **argv){
    gsea_args args;
    if(parse_args(argc, argv, &args)<0){ usage(); return 2; }
    if((args.do_encrypt || args.do_decrypt) && (!args.key || args.key_len==0)){
        write(2,"Falta clave (-k)\n",17);
        return 2;
    }
    if(run_jobs_concurrently(&args)<0) return 1;
    return 0;
}
