#include <stdio.h>
#include <stdlib.h>
#include "../lib/read_params/read_params.h"
#include "../lib/index_pthread/index_pthread.h"
#include "../lib/get_index/get_index.h"
#include "../models/node.h"
#include "../models/file_data.h"
#include <pthread.h>


int main(int argc, char ** argv) {
    int n;
    char * index_path, * dir_path;
    index_path = dir_path = NULL;
    if(read_params(argc, argv, &dir_path, &index_path, &n) != 0) return EXIT_FAILURE;
    node * head, *temp;
    head = temp = NULL;
    pthread_mutex_t mx_head = PTHREAD_MUTEX_INITIALIZER;

    index_args * args = malloc(sizeof(index_args));
    if(!args) {
        fprintf(stderr, "Malloc function failed\n");
        return EXIT_FAILURE;
    }
    args->head = &head;
    args->index_path = index_path;
    args->dir_path = dir_path;
    args->time = n;
    args->mx_head = &mx_head;
    if(pthread_create(&(args->id), NULL, index_thread_work, args) != 0) {
        fprintf(stderr, "Pthread_create function failed\n");
        return EXIT_FAILURE;
    }
    sleep(1);
    pthread_mutex_lock(&mx_head);
    temp = head;
    while(temp) {
        printf("Mamy plik %s\n", temp->elem.name);
        temp = temp->next;
    }
    pthread_mutex_unlock(&mx_head);
    free(args);
    return EXIT_SUCCESS;
}