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
#include "../index_pthread/index_pthread.h"
#include "../../models/node.h"
#include "../../models/file_data.h"
#include "../../utils/utils.h"
#define MAXFD 20
#define MAGIC_NUMBERS_BYTES 2
#define BITS_IN_BYTE 8
#define NFTW_STOPPED_ON_FLAG 111
node * head;
int * exit_flag;
int ile = 0;

static int insert(file new_file) {
    node * new_node;
    if(!(new_node = malloc(sizeof(node)))) FATAL("Malloc");

    new_node->next = head;
    new_node->elem = new_file;
    head = new_node;

    return EXIT_SUCCESS;
}

static file create_new_file(const char *path, const struct stat *s, int offset, unsigned int file_type) {
    file new_file;

    if ((new_file.name = malloc(sizeof(char) * (strlen(path) - offset + 1))) == NULL) FATAL("Malloc");
    strncpy(new_file.name, &path[offset], (strlen(path) - offset));
    new_file.name[strlen(path) - offset] = '\0';

    if ((new_file.path = realpath(path, NULL)) == NULL) FATAL("Realpath");

    new_file.owner_uid = s->st_uid;
    new_file.size = s->st_size;
    new_file.type = file_type;
    return new_file;
}

static int walk(const char *path, const struct stat *s, int type, struct FTW *f) {
    file new_file;
    if (*exit_flag == EXIT_NOW) return NFTW_STOPPED_ON_FLAG;

    if (type == FTW_D) {
        new_file = create_new_file(path, s, f->base, DIR);
        insert(new_file);
    }
    else {
        unsigned char magic_number[MAGIC_NUMBERS_BYTES];
        int in;
        if ((in = TEMP_FAILURE_RETRY(open(path, O_RDONLY))) < 0) FATAL("OPEN");
        if (bulk_read(in, magic_number, MAGIC_NUMBERS_BYTES) != MAGIC_NUMBERS_BYTES) FATAL("Bulk_read");
        
        unsigned int file_type = (magic_number[0] << BITS_IN_BYTE) + magic_number[1];
        if(file_type == JPEG || file_type == PNG || file_type == GZIP || file_type == ZIP) {
            new_file = create_new_file(path, s, f->base, file_type);
            insert(new_file);
        }

        if(TEMP_FAILURE_RETRY(close(in))) FATAL("Close");
    }
    return EXIT_SUCCESS;
}

int get_index(char * dir_path, node ** global_head, int * exit_flag_handler) {
    exit_flag = exit_flag_handler;
    head = NULL;
    int nftw_exit_status;
    if ((nftw_exit_status = nftw(dir_path, walk, MAXFD, FTW_PHYS)) != 0) {
        if (nftw_exit_status != NFTW_STOPPED_ON_FLAG) FATAL("Nftw");
    }

    if (*exit_flag == EXIT_NOW) free_old_list(head);
    else *global_head = head;

    return EXIT_SUCCESS;
}