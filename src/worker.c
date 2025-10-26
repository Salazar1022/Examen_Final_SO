#include "worker.h"
#include "fs.h"
#include "pipeline.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>   /* SOLO para mensajes a consola; el examen restringe stdio en archivos */
#include <string.h>
#include <errno.h>

typedef struct {
    const gsea_args *args;
    job_t job;
    sem_t *sem;
} task_t;

static void* worker(void *p){
    task_t *t = (task_t*)p;
    int rc = run_pipeline_file(t->args, t->job.path_in, t->job.path_out);
    if(rc<0){
        fprintf(stderr, "Error procesando %s -> %s (%s)\n",
                t->job.path_in, t->job.path_out, strerror(errno));
    }
    sem_post(t->sem);
    free(t);
    return NULL;
}

int run_jobs_concurrently(const gsea_args *args){
    job_t *jobs=NULL; size_t n=0;
    if(enumerate_jobs(args, &jobs, &n)<0) return -1;

    if(n==0){
        /* archivo único */
        int rc = run_pipeline_file(args, jobs[0].path_in, jobs[0].path_out);
        free(jobs);
        return rc;
    }

    sem_t sem;
    sem_init(&sem, 0, args->max_threads);

    pthread_t *tids = malloc(sizeof(pthread_t)*n);
    for(size_t i=0;i<n;i++){
        sem_wait(&sem);
        task_t *t = malloc(sizeof(task_t));
        t->args=args; t->job=jobs[i]; t->sem=&sem;
        pthread_create(&tids[i], NULL, worker, t);
    }
    for(size_t i=0;i<n;i++) pthread_join(tids[i], NULL);
    sem_destroy(&sem);
    free(tids); free(jobs);
    return 0;
}
