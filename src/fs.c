#include "fs.h"
#include "util.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
  #include <direct.h>      // _mkdir
  #include <io.h>
  #define MKDIR(path, mode)   _mkdir(path)        // ignora 'mode' en Windows
  #define STAT(path, stp)     _stat((path), (stp))
  #define S_ISDIR(m)          (((m) & _S_IFMT) == _S_IFDIR)
  typedef struct _stat stat_t;
#else
  #include <unistd.h>
  #define MKDIR(path, mode)   mkdir((path), (mode))
  #define STAT(path, stp)     stat((path), (stp))
  typedef struct stat stat_t;
#endif

static int is_dir(const char *p){
    struct stat st;
    if(stat(p,&st)<0) return 0;
    return S_ISDIR(st.st_mode);
}

static int ensure_dir(const char *p) {
    stat_t st;
    if (STAT(p, &st) == 0) {
        if (S_ISDIR(st.st_mode)) return 0;   // ya existe y es dir
        errno = ENOTDIR;
        return -1;                            // existe pero no es dir
    }

    // crear directorio (0755 solo se usa en POSIX; en Windows se ignora)
    if (MKDIR(p, 0755) < 0) {
        if (errno == EEXIST) return 0;       // ya existe (carrera)
        return -1;
    }
    return 0;
}

int enumerate_jobs(const gsea_args *args, job_t **jobs, size_t *count){
    *jobs=NULL; *count=0;
    if(!is_dir(args->in_path)){
        *jobs = xmalloc(sizeof(job_t));
        (*count)=1;
        strncpy((*jobs)[0].path_in, args->in_path, sizeof((*jobs)[0].path_in)-1);
        strncpy((*jobs)[0].path_out, args->out_path, sizeof((*jobs)[0].path_out)-1);
        return 0;
    }
    /* directorio */
    if(ensure_dir(args->out_path)<0 && errno!=EEXIST) return -1;
    DIR *d = opendir(args->in_path);
    if(!d) return -1;
    struct dirent *de;
    size_t cap=16, n=0;
    job_t *arr = xmalloc(cap*sizeof(job_t));
    while((de=readdir(d))){
        if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0) continue;
        char in[1024], out[1024];
        snprintf(in, sizeof(in), "%s/%s", args->in_path, de->d_name);
        if(is_dir(in)) continue; /* (opcional) recursivo: enumerar subdirectorios */
        snprintf(out,sizeof(out), "%s/%s", args->out_path, de->d_name);
        if(n==cap){ cap*=2; arr = realloc(arr, cap*sizeof(job_t)); }
        strncpy(arr[n].path_in, in, sizeof(arr[n].path_in)-1);
        strncpy(arr[n].path_out, out, sizeof(arr[n].path_out)-1);
        n++;
    }
    closedir(d);
    *jobs=arr; *count=n; return 0;
}

static int path_join(char *dst, size_t cap, const char *dir, const char *name){
    size_t ld = strlen(dir), ln = strlen(name);
    size_t need = ld + 1 /* '/' */ + ln + 1 /* '\0' */;
    if (need > cap) return -1;
    // copia dir
    for (size_t i=0;i<ld;i++) dst[i]=dir[i];
    dst[ld] = '/';
    // copia name
    for (size_t i=0;i<ln;i++) dst[ld+1+i]=name[i];
    dst[ld+1+ln] = '\0';

    return 0;
}
