#include <pthread.h>
#include <stdlib.h>
#include "../../models/node.h"

#ifndef _INDEX_PTHREAD_H
#define _INDEX_PTHREAD_H

typedef struct index_args {
    pthread_t id;
    unsigned int time;
    node ** head;
    pthread_mutex_t * mx_head;
    char * index_path;
    char * dir_path;
} index_args;

void * index_thread_work(void * raw_args);

#endif