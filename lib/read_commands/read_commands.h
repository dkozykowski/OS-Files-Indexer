#include <pthread.h>
#include "../../models/node.h"

#ifndef _READ_COMMANDS_H
#define _READ_COMMANDS_H

typedef struct read_commands_args
{
    pthread_mutex_t * mx_stdout;
    int * status_flag;
    pthread_mutex_t * mx_status_flag;
    int * exit_flag;
    pthread_mutex_t * mx_exit_flag;
    pthread_mutex_t *mx_file_saving_flag;
    pthread_mutex_t *mx_head;
    node ** head;
    char * pager;
} read_commands_args;


void read_commands(read_commands_args *args);

#endif