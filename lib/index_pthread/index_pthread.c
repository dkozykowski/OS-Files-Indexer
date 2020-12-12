#include "index_pthread.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include "../get_index/get_index.h"
#include "../data_saving/data_saving.h"
#include "../../models/node.h"
#include "../../models/file_data.h"
#define ELAPSED(start,end) ((end).tv_sec-(start).tv_sec)+(((end).tv_nsec - (start).tv_nsec) * 1.0e-9)

static void free_old_list(node * head) {
    node * temp;
    while(head) {
        temp = head;
        head = head->next;
        free(temp->elem.name);
        free(temp->elem.path);
        free(temp);
    }
}

void * index_thread_work(void * raw_args) {
    struct timespec last_edit_time, current_time;
    index_args * args = raw_args;
    struct stat filestat;
    node * new_head, * old_head;

    if(lstat(args->index_path, &filestat)) {
        if (errno == ENOENT) {
            if(get_index(args->dir_path, &new_head) != 0) exit(EXIT_FAILURE);
            save_data_to_file(args->index_path, new_head);
            pthread_mutex_lock(args->mx_head);
            old_head = *(args->head);
            *(args->head) = new_head;
            new_head = NULL;
            pthread_mutex_unlock(args->mx_head);
            free_old_list(old_head);
        }
        else {
           fprintf(stderr, "Lstat function failed");
           exit(EXIT_FAILURE); 
        }
    }
    last_edit_time = filestat.st_ctim;
    if(clock_gettime(CLOCK_REALTIME, &current_time)) {
        fprintf(stderr, "Clock_gettime function failed");
        exit(EXIT_FAILURE); 
    }

    if (args->time > 0 && ELAPSED(last_edit_time, current_time) >= args->time) {
        if(get_index(args->dir_path, &new_head) != 0) exit(EXIT_FAILURE);
        save_data_to_file(args->index_path, new_head);
        pthread_mutex_lock(args->mx_head);
        old_head = *(args->head);
        *(args->head) = new_head;
        new_head = NULL;
        pthread_mutex_unlock(args->mx_head);
        free_old_list(old_head);
    } 
    else {
        load_data_from_file(args->index_path, &new_head);
        pthread_mutex_lock(args->mx_head);
        old_head = *(args->head);
        *(args->head) = new_head;
        new_head = NULL;
        pthread_mutex_unlock(args->mx_head);
    }
    return NULL;
}