#include "read_commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "../index_pthread/index_pthread.h"
#define DIR 1
#define JPEG 2
#define PNG 3
#define GZIP 4
#define ZIP 5

static void count(read_commands_args *args) {
    node * temp;
    int counter[6];
    pthread_mutex_lock(args->mx_head);
    temp = args->head;
    while(temp) {
        counter[temp->elem.type]++;
        temp = temp->next;
    }
    pthread_mutex_unlock(args->mx_head);
    pthread_mutex_lock(args->mx_stdout);
    printf("Folders: %d\nJPEG: %d\nPNG: %d\nGZIP: %d\n ZIP: %d\n", 
        counter[1], counter[2], counter[3], counter[4], counter[5]);
    pthread_mutex_unlock(args->mx_stdout);
}

static void print_file_info(read_commands_args * args, file * elem) {
    pthread_mutex_lock(args->mx_stdout);
    printf("Path: %s\nSize: %ld\nType: ", (*elem).path, (*elem).size);
    switch((*elem).type) {
        case DIR: 
            printf("DIR\n\n");
            break;
        case JPEG:
            printf("JPEG\n\n");
            break;
        case PNG:
            printf("PNG\n\n");
            break;
        case GZIP:
            printf("GZIP\n\n");
            break;
        case ZIP:
            printf("ZIP\n\n");
            break;
        default:
            break;
    }
}

static void owner(read_commands_args * args, uid_t uid) {
    node * temp;
    pthread_mutex_lock(args->mx_head);
    temp = args->head;
    while(temp) {
        if (temp->elem.owner_uid == uid) {
            print_file_info(args, &(temp->elem));   
        }
        pthread_mutex_unlock(args->mx_stdout);
        temp = temp->next;
    }
    pthread_mutex_unlock(args->mx_head);
}

static void largerthan(read_commands_args * args, ssize_t size) {
    node * temp;
    pthread_mutex_lock(args->mx_head);
    temp = args->head;
    while(temp) {
        if (temp->elem.size > size) {
            print_file_info(args, &(temp->elem));   
        }
        pthread_mutex_unlock(args->mx_stdout);
        temp = temp->next;
    }
    pthread_mutex_unlock(args->mx_head);
}

static void namepart(read_commands_args * args, char * sequence) {
    node * temp;
    pthread_mutex_lock(args->mx_head);
    temp = args->head;
    while(temp) {
        if (strstr(temp->elem.name, sequence) != NULL) {
            print_file_info(args, &(temp->elem));
        }
        pthread_mutex_unlock(args->mx_stdout);
        temp = temp->next;
    }
    pthread_mutex_unlock(args->mx_head);
}

void read_commands(read_commands_args *args) {
    char command_name[20];
    char buffer[100];
    char argument[50];
    int status_flag;
    while(1) {
        fgets(buffer, 100, stdin);
        int arguments_read = sscanf(buffer, "%s %s", command_name, argument);
        if (arguments_read > 2) {
            fprintf(stderr, "Invalid argument\n");
            continue;
        };
        if (strcmp(command_name, "exit") == 0) {
            if (arguments_read != 1) {
                fprintf(stderr, "Invalid argument\n");
                continue;
            };
            pthread_mutex_lock(args->mx_exit_flag);
            *(args->exit_flag) = EXIT;
            pthread_mutex_unlock(args->mx_exit_flag);
            break;
        }
        else if (strcmp(command_name, "exit!") == 0) {
            if (arguments_read != 1) {
                fprintf(stderr, "Invalid argument\n");
                continue;
            };
            pthread_mutex_lock(args->mx_file_saving_flag);
            exit(EXIT_SUCCESS);
            break;
        }
        else if (strcmp(command_name, "index") == 0) {
            if (arguments_read != 1) {
                fprintf(stderr, "Invalid argument\n");
                continue;
            };
            pthread_mutex_lock(args->mx_status_flag);
            status_flag = *(args->status_flag);
            if (*args->status_flag == DONE) *(args->status_flag) = PENDING;
            pthread_mutex_unlock(args->mx_status_flag);
            if (status_flag != DONE) {
                pthread_mutex_lock(args->mx_stdout);
                printf("Index process already running\n");
                pthread_mutex_unlock(args->mx_stdout);
            }
        }
        else if (strcmp(command_name, "count") == 0) {
            if (arguments_read != 1) {
                fprintf(stderr, "Invalid argument\n");
                continue;
            };
            count(args);
        }
        else if (strcmp(command_name, "largerthan") == 0) {
            if (arguments_read != 2) {
                fprintf(stderr, "Invalid argument\n");
                continue;
            };
            ssize_t file_size = atoi(argument);
            largerthan(args, file_size);
        }
        else if (strcmp(command_name, "namepart") == 0) {
            if (arguments_read != 2) {
                fprintf(stderr, "Invalid argument\n");
                continue;
            };
            namepart(args, argument);
        }
        else if (strcmp(command_name, "owner") == 0) {
            if (arguments_read != 2) {
                fprintf(stderr, "Invalid argument\n");
                continue;
            };
            __uid_t owner_id = atoi(argument);
            owner(args, owner_id);
        }
        else {
            fprintf(stderr, "Invalid argument\n");
        };
    }
    
}