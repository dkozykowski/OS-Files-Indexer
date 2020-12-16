#define _GNU_SOURCE
#include "data_saving.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../bulk_operations/bulk_operations.h"
#include "../../utils/utils.h"

int save_data_to_file(char * path, node * head) {
    int out;
    if ((out = TEMP_FAILURE_RETRY(open(path, O_CREAT|O_TRUNC|O_WRONLY, 0777))) < 0) 
        FATAL("Open");

    int temp;
    node * temp_node = head;
    while(temp_node) {
        temp = strlen(temp_node->elem.name);
        if (bulk_write(out, &temp, sizeof(int)) < 0) FATAL("Bulk_write");
        if (bulk_write(out, temp_node->elem.name, temp) < 0) FATAL("Bulk_write");

        temp = strlen(temp_node->elem.path);
        if (bulk_write(out, &temp, sizeof(int)) < 0) FATAL("Bulk_write");
        if (bulk_write(out, temp_node->elem.path, temp) < 0) FATAL("Bulk_write");

        if (bulk_write(out, &(temp_node->elem.owner_uid), sizeof(unsigned int)) < 0) FATAL("Bulk_write");
        if (bulk_write(out, &(temp_node->elem.size), sizeof(ssize_t)) < 0) FATAL("Bulk_write");
        if (bulk_write(out, &(temp_node->elem.type), sizeof(int)) < 0) FATAL("Bulk_write");
        
        temp_node = temp_node->next;
    }
    
    if(TEMP_FAILURE_RETRY(close(out))) FATAL("Close");
    return EXIT_SUCCESS;
}

int load_data_from_file(char * path, node ** head) {
    int in;
    if ((in = TEMP_FAILURE_RETRY(open(path, O_RDONLY))) < 0) FATAL("Open");

    file new_file;
    int temp;
    int c;

    while(1) {
        c = bulk_read(in, &temp, sizeof(int));
        if (c == 0) return EXIT_SUCCESS;
        if (c < 0) FATAL("Bulk_read");
        new_file.name = malloc(sizeof(char) * (temp + 1));
        if(!new_file.name) FATAL("Malloc");
        if (bulk_read(in, new_file.name, temp) < 0) FATAL("Bulk_read");
        new_file.name[temp] = '\0';

        if (bulk_read(in, &temp, sizeof(int)) < 0) FATAL("Bulk_read");
        new_file.path = malloc(sizeof(char) * (temp + 1));
        if (!new_file.path) FATAL("Malloc");
        if (bulk_read(in, new_file.path, temp) < 0) FATAL("Bulk_read");
        new_file.path[temp] = '\0';

        if (bulk_read(in, &new_file.owner_uid, sizeof(unsigned int)) < 0) FATAL("Bulk_read");
        if (bulk_read(in, &new_file.size, sizeof(ssize_t)) < 0) FATAL("Bulk_read");
        if (bulk_read(in, &new_file.type, sizeof(int)) < 0) FATAL("Bulk_read");

        node * new_node = malloc(sizeof(node));
        if (!new_node) FATAL("Malloc");
        new_node->next = *head;
        new_node->elem = new_file;
        (*head) = new_node;
    }

    if(TEMP_FAILURE_RETRY(close(in))) FATAL("Close");
    return EXIT_SUCCESS;
}