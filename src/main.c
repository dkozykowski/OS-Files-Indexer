#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "../lib/read_params/read_params.h"
#include "../lib/index_pthread/index_pthread.h"
#include "../lib/get_index/get_index.h"
#include "../lib/read_commands/read_commands.h"
#include "../models/node.h"
#include "../utils/utils.h"
#include "../models/file_data.h"

int main(int argc, char ** argv) {
    int n, default_index_path = 0;
    char * index_path, * dir_path;
    index_path = dir_path = NULL;
    if(read_params(argc, argv, &dir_path, &index_path, &n, &default_index_path) != 0) FATAL("Read_params");
    node * head, *temp;
    head = temp = NULL;
    pthread_mutex_t mx_head = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mx_stdout = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mx_status_flag = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mx_exit_flag = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mx_file_saving_flag = PTHREAD_MUTEX_INITIALIZER;

    index_args * args_index = malloc(sizeof(index_args));
    if(!args_index) FATAL("Malloc");
    args_index->mx_stdout = &mx_stdout;
    args_index->head = &head;
    args_index->index_path = index_path;
    args_index->dir_path = dir_path;
    args_index->time = n;
    args_index->mx_head = &mx_head;
    args_index->status_flag = DONE;
    args_index->mx_status_flag = &mx_status_flag;
    args_index->mx_exit_flag = &mx_exit_flag;
    args_index->exit_flag = NONE;
    args_index->mx_file_saving_flag = &mx_file_saving_flag;
    if(pthread_create(&(args_index->id), NULL, index_thread_work, args_index) != 0) FATAL("Pthread_create");

    read_commands_args * args_r_c;
    args_r_c = malloc(sizeof(read_commands_args));
    if(!args_r_c) FATAL("Malloc");
    args_r_c->mx_status_flag = &mx_status_flag;
    args_r_c->mx_stdout = &mx_stdout;
    args_r_c->mx_status_flag = &mx_status_flag;
    args_r_c->exit_flag = &(args_index->exit_flag);
    args_r_c->mx_exit_flag = &mx_exit_flag;
    args_r_c->status_flag = &(args_index->status_flag);
    args_r_c->head = &head;
    args_r_c->mx_head = &mx_head;
    args_r_c->mx_file_saving_flag = &mx_file_saving_flag;
    args_r_c->pager = getenv("PAGER");
    args_r_c->indexing_thread_id = args_index->id;
    
    read_commands(args_r_c);

    pthread_join(args_index->id, NULL);
    free(args_index);
    free(args_r_c);
    free_old_list(head);
    if (default_index_path) free(index_path);
    return EXIT_SUCCESS;
}