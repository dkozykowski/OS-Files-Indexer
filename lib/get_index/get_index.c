#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include "get_index.h"
#include <unistd.h>
#include "errno.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ftw.h>
#include <string.h>
#include "../bulk_operations/bulk_operations.h"
#include "../../models/node.h"
#include "../../models/file_data.h"
#define MAXFD 20
node * head;
int * exit_flag;
int ile = 0;

static int insert(file new_file) {
    node * new_node;
    if(!(new_node = malloc(sizeof(node)))) {
        fprintf(stderr, "Malloc function failed\n");
        return EXIT_FAILURE;
    }
    new_node->next = head;
    new_node->elem = new_file;
    head = new_node;
    return EXIT_SUCCESS;
}

static file create_new_file(const char *path, const struct stat *s, int offset, unsigned int file_type) {
    file new_file;
    new_file.name = malloc(sizeof(char) * (strlen(path) - offset + 1));
    strncpy(new_file.name, &path[offset], (strlen(path) - offset));
    for (int i = offset, o = 0; i < strlen(path); i++, o++) {
        new_file.name[o] = path[i];
    }
    new_file.name[strlen(path) - offset] = '\0';
    new_file.owner_uid = s->st_uid;
    new_file.path = realpath(path, NULL);
    if (new_file.name == NULL) {
        fprintf(stderr, "Realpath function failed\n");
        exit(EXIT_FAILURE);
    }
    new_file.size = s->st_size;
    new_file.type = file_type;
    return new_file;
}

static int walk(const char *path, const struct stat *s, int type, struct FTW *f) {
    file new_file;
    if (type == FTW_D) {
        new_file = create_new_file(path, s, f->base, DIR);
        insert(new_file);
    }
    else {
        unsigned char magic_number[2];
        int in;
        if ((in = TEMP_FAILURE_RETRY(open(path, O_RDONLY))) < 0) {
            fprintf(stderr, "Open function failed\n");
            return EXIT_FAILURE;
        }
        if (bulk_read(in, magic_number, 2) != 2) {
            fprintf(stderr, "Open function failed\n");
            return EXIT_FAILURE;
        }
        
        unsigned int file_type = magic_number[0] * 16 * 16 + magic_number[1];
        if(file_type == JPEG || file_type == PNG || file_type == GZIP || file_type == ZIP) {
            new_file = create_new_file(path, s, f->base, file_type);
            insert(new_file);
        }
        if((close(in))) {
            fprintf(stderr, "Close function failed\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int get_index(char * dir_path, node ** global_head) {
    head = NULL;
    if (nftw(dir_path, walk, MAXFD, FTW_PHYS) != 0) {
        fprintf(stderr, "Nftw function failed\n");
        return EXIT_FAILURE;
    }
    *global_head = head;
    return EXIT_SUCCESS;
}