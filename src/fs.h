#pragma once
#include "cli.h"
#include <sys/types.h>

typedef struct {
    char path_in[1024];
    char path_out[1024];
} job_t;

int enumerate_jobs(const gsea_args *args, job_t **jobs, size_t *count);
/* crea out_path si args->in_path es archivo único (out es archivo) o
   si es directorio (out es directorio; se crea si no existe) */
