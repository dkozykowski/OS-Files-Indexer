#include <pthread.h>
#include <stdlib.h>
#include "../../models/node.h"
#define PENDING 1
#define IN_PROGRESS 2
#define DONE 3

#define NONE 1
#define EXIT 2
#define EXIT_NOW 3

#ifndef _INDEX_PTHREAD_H
#define _INDEX_PTHREAD_H

typedef struct index_args {
    pthread_t id;
    int time;
    node ** head;
    pthread_mutex_t * mx_head;
    char * index_path;
    char * dir_path;
    pthread_mutex_t * mx_stdout;
    int status_flag;
    pthread_mutex_t * mx_status_flag;
    int exit_flag;
    pthread_mutex_t * mx_exit_flag;
    pthread_mutex_t *mx_file_saving_flag;
} index_args;

void * index_thread_work(void * raw_args);

#endif