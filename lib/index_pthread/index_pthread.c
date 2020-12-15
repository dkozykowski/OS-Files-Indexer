#include "index_pthread.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "../get_index/get_index.h"
#include "../data_saving/data_saving.h"
#include "../../models/node.h"
#include "../../models/file_data.h"
#define ELAPSED(start,end) ((end).tv_sec-(start).tv_sec)

static void free_old_list(node * head) {
    node * temp;
    while(head) {
        temp = head;
        head = head->next;
        if(temp->elem.name) free(temp->elem.name);
        if (temp->elem.path) free(temp->elem.path);
        free(temp);
    }
}

static void perform_indexing(index_args * args, int * exit_flag) {
    node * new_head, * old_head;
    old_head = new_head = NULL;
    pthread_mutex_lock(args->mx_status_flag);
    args->status_flag = IN_PROGRESS;
    pthread_mutex_unlock(args->mx_status_flag);

    if(get_index(args->dir_path, &new_head, exit_flag) != 0) exit(EXIT_FAILURE);
    pthread_mutex_lock(args->mx_file_saving_flag);
    if (save_data_to_file(args->index_path, new_head) != 0) {
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(args->mx_file_saving_flag);

    pthread_mutex_lock(args->mx_stdout);
    printf("\n[Indexer] Indexing process has just been finished\n");
    pthread_mutex_unlock(args->mx_stdout);

    pthread_mutex_lock(args->mx_head);
    old_head = *(args->head);
    *(args->head) = new_head;
    new_head = NULL;
    pthread_mutex_unlock(args->mx_head);

    pthread_mutex_lock(args->mx_status_flag);
    args->status_flag = DONE;
    pthread_mutex_unlock(args->mx_status_flag);

    free_old_list(old_head);
    old_head = NULL;
}

void * index_thread_work(void * raw_args) {
    struct timespec last_edit_time, current_time;
    index_args * args = raw_args;
    struct stat filestat;
    int status_flag = DONE;
    int exit_flag = NONE;
    

    if(lstat(args->index_path, &filestat)) {
        if (errno == ENOENT) {
            perform_indexing(args, &exit_flag);
            clock_gettime(CLOCK_REALTIME, &last_edit_time);
            if(args->time > 0) alarm(args->time);
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
        perform_indexing(args, &exit_flag);
        clock_gettime(CLOCK_REALTIME, &last_edit_time);
        alarm(args->time);
    } 
    else {
        node * new_head, * old_head;
        new_head = NULL;
        if (args->time > 0) {
            alarm(args->time - ELAPSED(last_edit_time, current_time));
        }
        load_data_from_file(args->index_path, &new_head);
        pthread_mutex_lock(args->mx_head);
        old_head = *(args->head);
        *(args->head) = new_head;
        new_head = NULL;
        pthread_mutex_unlock(args->mx_head);

        free_old_list(old_head);
        old_head = NULL;
    }


    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGALRM);
    sigaddset(&signals, SIGUSR1);

    int signo;
    while(1) {
        if (sigwait(&signals, &signo) != 0) {
            fprintf(stderr, "Sigwait function failed\n");
            exit(EXIT_FAILURE);
        }
        if(args->time > 0) {
            clock_gettime(CLOCK_REALTIME, &current_time);
            if (ELAPSED(last_edit_time, current_time) >= args->time) {
                if(clock_gettime(CLOCK_REALTIME, &current_time)) {
                    fprintf(stderr, "Clock_gettime function failed");
                    exit(EXIT_FAILURE); 
                }
                perform_indexing(args, &exit_flag);
                clock_gettime(CLOCK_REALTIME, &last_edit_time);
                alarm(args->time);
            }
        }
        else {
            pthread_mutex_lock(args->mx_status_flag);
            if(args->status_flag == PENDING) {
                status_flag = PENDING;
                args->status_flag = IN_PROGRESS;
            }
            pthread_mutex_unlock(args->mx_status_flag);
            if (status_flag == PENDING) {
                perform_indexing(args, &exit_flag);
                clock_gettime(CLOCK_REALTIME, &last_edit_time);
                if(args->time > 0) alarm(args->time);
            }
            status_flag = DONE;
            pthread_mutex_lock(args->mx_status_flag);
            args->status_flag = DONE;
            pthread_mutex_unlock(args->mx_status_flag);
        }
        pthread_mutex_lock(args->mx_exit_flag);
        exit_flag = args->exit_flag;
        pthread_mutex_unlock(args->mx_exit_flag);
        if (exit_flag == EXIT || exit_flag == EXIT_NOW) break;
    }
    return NULL;
}