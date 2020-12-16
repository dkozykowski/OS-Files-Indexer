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
#include "../../utils/utils.h"

static void perform_indexing(index_args * args, int * exit_flag) {
    node * new_head, * old_head;
    old_head = new_head = NULL;

    pthread_mutex_lock(args->mx_status_flag); {
        args->status_flag = IN_PROGRESS;
    } pthread_mutex_unlock(args->mx_status_flag);

    if(get_index(args->dir_path, &new_head, exit_flag) != 0) FATAL("Get_index");

    pthread_mutex_lock(args->mx_file_saving_flag); {
        if (save_data_to_file(args->index_path, new_head) != 0) FATAL("Save_data_to_file");
    } pthread_mutex_unlock(args->mx_file_saving_flag);

    pthread_mutex_lock(args->mx_stdout); {
        printf("\n[Indexer] Indexing process has just been finished\n");
    } pthread_mutex_unlock(args->mx_stdout);

    pthread_mutex_lock(args->mx_head); {
        old_head = *(args->head);
        *(args->head) = new_head;
        new_head = NULL;
    } pthread_mutex_unlock(args->mx_head);

    pthread_mutex_lock(args->mx_status_flag); {
        args->status_flag = DONE;
    } pthread_mutex_unlock(args->mx_status_flag);

    free_old_list(old_head);
    old_head = NULL;
}

static void try_to_reload_index_from_file(
    index_args * args, 
    int * exit_flag, 
    struct timespec * last_indexing_time, 
    struct timespec * current_time
) {
    node * new_head = NULL, * old_head = NULL;
    struct stat filestat;

    if(lstat(args->index_path, &filestat)) {
        if (errno == ENOENT) {
            perform_indexing(args, exit_flag);
            clock_gettime(CLOCK_REALTIME, last_indexing_time);
            if(args->time > 0) alarm(args->time);
        }
        else FATAL("Lstat");
    }
    else {
        *last_indexing_time = filestat.st_ctim;

        if(clock_gettime(CLOCK_REALTIME, current_time)) FATAL("Clock_gettime");
        if (args->time > 0 && ELAPSED(*last_indexing_time, *current_time) >= args->time) {
            perform_indexing(args, exit_flag);
            clock_gettime(CLOCK_REALTIME, last_indexing_time);
            alarm(args->time);
        } 
        else {
            if (args->time > 0) alarm(args->time - ELAPSED(*last_indexing_time, *current_time));

            load_data_from_file(args->index_path, &new_head); 

            pthread_mutex_lock(args->mx_head); {
                old_head = *(args->head);
                *(args->head) = new_head;
                free_old_list(old_head);
                new_head = old_head = NULL;
            } pthread_mutex_unlock(args->mx_head);

            free_old_list(old_head);
            old_head = NULL;
        }
    }
}

static void update_global_status_glag(pthread_mutex_t * mx_status_flag, int * global_status_flag, int * local_status_flag) {
    pthread_mutex_lock(mx_status_flag); {
        *global_status_flag = *local_status_flag;
    } pthread_mutex_unlock(mx_status_flag);
}

static void work_on_signals(
    index_args * args,
    int * exit_flag, 
    int * status_flag,
    struct timespec * last_indexing_time, 
    struct timespec * current_time
) {
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGALRM);
    sigaddset(&signals, SIGUSR1);

    int signo;
    while(1) {
        if (sigwait(&signals, &signo) != 0) FATAL("Sigwait");
        if(args->time > 0) {
            if(clock_gettime(CLOCK_REALTIME, current_time)) FATAL("Clock_gettime");
            if (ELAPSED(*last_indexing_time, *current_time) >= args->time) {
                *status_flag = IN_PROGRESS;
                update_global_status_glag(args->mx_status_flag, &args->status_flag, status_flag);
                
                perform_indexing(args, exit_flag);

                *status_flag = DONE;
                update_global_status_glag(args->mx_status_flag, &args->status_flag, status_flag);

                clock_gettime(CLOCK_REALTIME, last_indexing_time);
                alarm(args->time);
            }
        }
        else {
            pthread_mutex_lock(args->mx_status_flag); {
                if(args->status_flag == PENDING) {
                    *status_flag = PENDING;
                    args->status_flag = IN_PROGRESS;
                }
            } pthread_mutex_unlock(args->mx_status_flag);

            if (*status_flag == PENDING) {
                *status_flag = IN_PROGRESS;
                perform_indexing(args, exit_flag);
                clock_gettime(CLOCK_REALTIME, last_indexing_time);
                if(args->time > 0) alarm(args->time);
            }
            *status_flag = DONE;
            update_global_status_glag(args->mx_status_flag, &args->status_flag, status_flag);
        }
        
        pthread_mutex_lock(args->mx_exit_flag); {
            *exit_flag = args->exit_flag;
        } pthread_mutex_unlock(args->mx_exit_flag);
        if (*exit_flag == EXIT || *exit_flag == EXIT_NOW) break;
    }
}

void * index_thread_work(void * raw_args) {
    struct timespec last_indexing_time, current_time;
    index_args * args = raw_args;
    int status_flag = DONE;
    int exit_flag = NONE;
    
    try_to_reload_index_from_file(args, &exit_flag, &last_indexing_time, &current_time);
    work_on_signals(args, &exit_flag, &status_flag, &last_indexing_time, &current_time);
    return NULL;
}