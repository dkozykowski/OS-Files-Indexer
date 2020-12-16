#include "read_commands.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include "../index_pthread/index_pthread.h"
#include "../get_index/get_index.h"
#include "../../utils/utils.h"
#define MAX_ELEMS_WITHOUT_PAGER 3
#define MAX_COMMAND_LENGH 20
#define MAX_ARGUMENT_LENGH 50
#define BUFFER_SIZE 100
#define MAX_COMMAND_LENGH_TEXT "20"
#define MAX_ARGUMENT_LENGH_TEXT "50"

static void count(read_commands_args *args) {
    int dir_counter = 0, jpeg_counter = 0, png_counter = 0, gzip_counter = 0, zip_counter = 0;
    pthread_mutex_lock(args->mx_head); {
        node * temp = *(args->head);
        while(temp) {
            switch(temp->elem.type) {
            case DIR: 
                dir_counter++; break;
            case JPEG:
                jpeg_counter++; break;
            case PNG:
                png_counter++; break;
            case GZIP:
                gzip_counter++; break;
            case ZIP:
                zip_counter++; break;
            default: break;
            }
            temp = temp->next;
        }
    } pthread_mutex_unlock(args->mx_head);
    
    pthread_mutex_lock(args->mx_stdout); {
        printf("DIRS: %d\nJPEG: %d\nPNG: %d\nGZIP: %d\nZIP: %d\n\n", 
        dir_counter, jpeg_counter, png_counter, gzip_counter, zip_counter);
    } pthread_mutex_unlock(args->mx_stdout);
}

static void print_single_file_info(read_commands_args * args, file * elem, FILE * output_destination) {
    fprintf(output_destination, "Path: %s\nSize: %ld\nType: ", (*elem).path, (*elem).size);
    switch((*elem).type) {
        case DIR: 
            fprintf(output_destination, "DIR\n\n"); break;
        case JPEG:
            fprintf(output_destination, "JPEG\n\n"); break;
        case PNG:
            fprintf(output_destination, "PNG\n\n"); break;
        case GZIP:
            fprintf(output_destination, "GZIP\n\n"); break;
        case ZIP:
            fprintf(output_destination, "ZIP\n\n"); break;
        default: break;
    }
}

static void print_list_elements(read_commands_args * args, int (*predicate)(node *, void *), void * predicate_param, FILE * output_destination) {
    node * temp;
    temp = *(args->head);
    while(temp) {
        if (predicate(temp, predicate_param)) {
            print_single_file_info(args, &(temp->elem), output_destination);   
        }
        temp = temp->next;
    }
}

static FILE * check_output_destination(node * head, int (*predicate)(node *, void *), void * predicate_param, char * pager, pthread_mutex_t * mx_stdout) {
    if (pager == NULL) {
        pthread_mutex_lock(mx_stdout);
        return stdout;
    }

    node * temp = head;
    int elements_counter = 0;
    while(temp) {
        if (predicate(temp, predicate_param)) elements_counter++;
        temp = temp->next;
    }
    if (elements_counter > MAX_ELEMS_WITHOUT_PAGER) {
        FILE * output_destination = popen(pager, "w");
        if (output_destination == NULL) FATAL("Popen");
        return output_destination;
    }
    else {
        pthread_mutex_lock(mx_stdout);
        return stdout;
    }
}

static void close_output_destination(FILE * output_destination, pthread_mutex_t * mx_stdout) {
    if (output_destination == stdout) {
        pthread_mutex_unlock(mx_stdout);
    } 
    else {
        if(pclose(output_destination) != 0) FATAL("Pclose");
    }
}

static int owner_predicate(node * head, void * owner_id) {
    return (head->elem.owner_uid == *((uid_t *) owner_id));
}

static void owner(read_commands_args * args, uid_t uid) {
    FILE * output_destination;
    pthread_mutex_lock(args->mx_head); {
        output_destination = check_output_destination(*(args->head), owner_predicate, &uid, args->pager, args->mx_stdout);
        print_list_elements(args, owner_predicate, &uid, output_destination);
        close_output_destination(output_destination, args->mx_stdout);
    } pthread_mutex_unlock(args->mx_head);
}

static int largerthan_predicate(node * head, void * file_size) {
    return (head->elem.size > *((ssize_t *)file_size));
}

static void largerthan(read_commands_args * args, ssize_t size) {
    FILE * output_destination;
    pthread_mutex_lock(args->mx_head); {
        output_destination = check_output_destination(*(args->head), largerthan_predicate, &size, args->pager, args->mx_stdout);
        print_list_elements(args, largerthan_predicate, &size, output_destination);
        close_output_destination(output_destination, args->mx_stdout);
    } pthread_mutex_unlock(args->mx_head);
}

static int namepart_predicate(node * head, void * sequence) {
    return (strstr(head->elem.name, (char *)sequence) != NULL);
}

static void namepart(read_commands_args * args, char * sequence) {
    FILE * output_destination;
    pthread_mutex_lock(args->mx_head); {
        output_destination = check_output_destination(*(args->head), namepart_predicate, sequence, args->pager, args->mx_stdout);
        print_list_elements(args, namepart_predicate, sequence, output_destination);
        close_output_destination(output_destination, args->mx_stdout);
    }
    pthread_mutex_unlock(args->mx_head);
}

static void m_exit(read_commands_args *args) {
    pthread_mutex_lock(args->mx_exit_flag); {
        *(args->exit_flag) = EXIT;
    } pthread_mutex_unlock(args->mx_exit_flag);

    if(kill(getpid(), SIGUSR1) != 0) FATAL("kill");
}

static void exit_now(read_commands_args *args) {
    pthread_mutex_lock(args->mx_exit_flag); {
        *(args->exit_flag) = EXIT_NOW;
    } pthread_mutex_unlock(args->mx_exit_flag);
    
    if(kill(getpid(), SIGUSR1) != 0) FATAL("Kill");
}

static void m_index(read_commands_args *args, int * status_flag) {
    pthread_mutex_lock(args->mx_status_flag); {
        *status_flag = *(args->status_flag);
        if (*args->status_flag == DONE) *(args->status_flag) = PENDING;
    } pthread_mutex_unlock(args->mx_status_flag);
    
    if (*status_flag != DONE) {
        pthread_mutex_lock(args->mx_stdout); {
            printf("Index process already running\n");
        } pthread_mutex_unlock(args->mx_stdout);
    }

    if(kill(getpid(), SIGUSR1) != 0) FATAL("Kill");
}

void read_commands(read_commands_args *args) {
    char command_name[MAX_COMMAND_LENGH + 1];
    char buffer[BUFFER_SIZE + 1];
    char argument[MAX_ARGUMENT_LENGH + 1];
    int status_flag;
    
    while(1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        int arguments_read = 
            sscanf(buffer, "%"MAX_COMMAND_LENGH_TEXT"s %"MAX_ARGUMENT_LENGH_TEXT"s", command_name, argument);
        if (strcmp(command_name, "exit") == 0) {
            if (arguments_read != 1) { ERR("Invalid argument"); continue; }
            m_exit(args);
            break;
        }
        else if (strcmp(command_name, "exit!") == 0) {
            if (arguments_read != 1) { ERR("Invalid argument"); continue; }
            exit_now(args);
            break;
        }
        else if (strcmp(command_name, "index") == 0) {
            if (arguments_read != 1) { ERR("Invalid argument"); continue; }
            m_index(args, &status_flag);
        }
        else if (strcmp(command_name, "count") == 0) {
            if (arguments_read != 1) { ERR("Invalid argument"); continue; }
            count(args);
        }
        else if (strcmp(command_name, "largerthan") == 0) {
            if (arguments_read != 2) { ERR("Invalid argument"); continue; }
            ssize_t file_size = atoi(argument);
            largerthan(args, file_size);
        }
        else if (strcmp(command_name, "namepart") == 0) {
            if (arguments_read != 2) { ERR("Invalid argument"); continue; }
            namepart(args, argument);
        }
        else if (strcmp(command_name, "owner") == 0) {
            if (arguments_read != 2) { ERR("Invalid argument"); continue; }
            __uid_t owner_id = atoi(argument);
            owner(args, owner_id);
        }
        else ERR("Invalid argument");
    }
    
}