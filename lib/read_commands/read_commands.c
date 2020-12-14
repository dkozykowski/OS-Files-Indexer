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

#define MAX_ELEMS_WITHOUT_PAGER 3

static void count(read_commands_args *args) {
    node * temp;
    int counter[6] = { 0, 0, 0, 0, 0, 0 };
    pthread_mutex_lock(args->mx_head);
    temp = *(args->head);
    while(temp) {
        counter[temp->elem.type]++;
        temp = temp->next;
    }
    pthread_mutex_unlock(args->mx_head);

    pthread_mutex_lock(args->mx_stdout);
    printf("Folders: %d\nJPEG: %d\nPNG: %d\nGZIP: %d\nZIP: %d\n\n", 
        counter[1], counter[2], counter[3], counter[4], counter[5]);
    pthread_mutex_unlock(args->mx_stdout);
}

static void print_file_info(read_commands_args * args, file * elem, FILE * output_destination) {
    if (output_destination == stdout) pthread_mutex_lock(args->mx_stdout);
    fprintf(output_destination, "Path: %s\nSize: %ld\nType: ", (*elem).path, (*elem).size);
    switch((*elem).type) {
        case DIR: 
            fprintf(output_destination, "DIR\n\n");
            break;
        case JPEG:
            fprintf(output_destination, "JPEG\n\n");
            break;
        case PNG:
            fprintf(output_destination, "PNG\n\n");
            break;
        case GZIP:
            fprintf(output_destination, "GZIP\n\n");
            break;
        case ZIP:
            fprintf(output_destination, "ZIP\n\n");
            break;
        default:
            break;
    }
    if (output_destination == stdout) pthread_mutex_unlock(args->mx_stdout);
}

static void owner(read_commands_args * args, uid_t uid) {
    node * temp;
    FILE * output_destination;
    int elements_counter = 0;
    pthread_mutex_lock(args->mx_head);
    temp = *(args->head);
    while(temp) {
        if (temp->elem.owner_uid == uid) elements_counter++;
        temp = temp->next;
    }

    if (elements_counter > MAX_ELEMS_WITHOUT_PAGER && args->pager) {
        output_destination = popen(args->pager, "w");
        if (output_destination == NULL) {
            fprintf(stderr, "Popen function failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else output_destination = stdout;

    temp = *(args->head);
    while(temp) {
        if (temp->elem.owner_uid == uid) {
            print_file_info(args, &(temp->elem), output_destination);   
        }
        temp = temp->next;
    }
    if (output_destination != stdout) {
        if(pclose(output_destination) != 0) {
            fprintf(stderr, "Pclose function failed\n");
        }
    }
    pthread_mutex_unlock(args->mx_head);
}

static void largerthan(read_commands_args * args, ssize_t size) {
    node * temp;
    FILE * output_destination;
    int elements_counter = 0;
    pthread_mutex_lock(args->mx_head);

    temp = *(args->head);
    while(temp) {
        if (temp->elem.size > size) elements_counter++;
        temp = temp->next;
    }
    
    if (elements_counter > MAX_ELEMS_WITHOUT_PAGER && args->pager) {
        output_destination = popen(args->pager, "w");
        if (output_destination == NULL) {
            fprintf(stderr, "Popen function failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else output_destination = stdout;

    temp = *(args->head);
    while(temp) {
        if (temp->elem.size > size) {
            print_file_info(args, &(temp->elem), output_destination);   
        }
        temp = temp->next;
    }
    if (output_destination != stdout) {
        if(pclose(output_destination) != 0) {
            fprintf(stderr, "Pclose function failed\n");
        }
    }
    pthread_mutex_unlock(args->mx_head);
}

static void namepart(read_commands_args * args, char * sequence) {
    node * temp;
    FILE * output_destination;
    int elements_counter = 0;
    pthread_mutex_lock(args->mx_head);

    temp = *(args->head);
    while(temp) {
        if (strstr(temp->elem.name, sequence) != NULL) elements_counter++;
        temp = temp->next;
    }

    if (elements_counter > MAX_ELEMS_WITHOUT_PAGER && args->pager) {
        output_destination = popen(args->pager, "w");
        if (output_destination == NULL) {
            fprintf(stderr, "Popen function failed\n");
            exit(EXIT_FAILURE);
        }
    }
    else output_destination = stdout;

    temp = *(args->head);
    while(temp) {
        if (strstr(temp->elem.name, sequence) != NULL) {
            print_file_info(args, &(temp->elem), output_destination);
        }
        temp = temp->next;
    }
    if (output_destination != stdout) {
        if(pclose(output_destination) != 0) {
            fprintf(stderr, "Pclose function failed\n");
        }
    }
    pthread_mutex_unlock(args->mx_head);
}

void read_commands(read_commands_args *args) {
    char command_name[21];
    char buffer[101];
    char argument[51];
    int status_flag;
    while(1) {
        fgets(buffer, 100, stdin);
        int arguments_read = sscanf(buffer, "%20s %50s", command_name, argument);
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