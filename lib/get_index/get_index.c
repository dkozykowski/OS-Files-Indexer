#define _XOPEN_SOURCE 500
#include "get_index.h"
#include "errno.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ftw.h>
#include <string.h>
#define MAXFD 20
#include "../../models/node.h"
#include "../../models/file_data.h"
#define JPEG 65496
#define PNG 35152
#define GZIP 8075
#define ZIP 20555
node * head;
// podmien potem spowrotem head i usun zeby nie bylo wyciekow pamieci 

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
    new_file.name = malloc(sizeof(char) * (strlen(path) - offset) + 1);
    strncpy(new_file.name, &path[offset], (strlen(path) - offset));
    for (int i = offset, o = 0; i < strlen(path); i++, o++) {
        new_file.name[o] = path[i];
    }
    new_file.name[strlen(path) - offset] = '\0';
    new_file.owner_uid = s->st_uid;
    new_file.path = malloc(sizeof(char) * strlen(path) + 1);
    strcpy(new_file.path, path);
    new_file.size = s->st_size;
    new_file.type = 1;
    return new_file;
}

static int walk(const char *path, const struct stat *s, int type, struct FTW *f) {
    file new_file;
    if (type == FTW_D) {
        new_file = create_new_file(path, s, f->base, 1);
        insert(new_file);
    }
    else {
        unsigned char magic_number[2];
        int in;
        if ((in = open(path, O_RDONLY)) < 0) {
            fprintf(stderr, "Open function failed\n");
            return EXIT_FAILURE;
        }
        if (read(in, magic_number, 2) != 2) {
            fprintf(stderr, "Open function failed\n");
            return EXIT_FAILURE;
        }
        
        unsigned int file_type = magic_number[0] * 16 * 16 + magic_number[1];
        switch(file_type) {
            case JPEG: 
                new_file = create_new_file(path, s, f->base, 2);
                insert(new_file);
                break;
            case PNG: 
                new_file = create_new_file(path, s, f->base, 3);
                insert(new_file);
                break;
            case GZIP:
                new_file = create_new_file(path, s, f->base, 4);
                insert(new_file);
                break;
            case ZIP: 
                new_file = create_new_file(path, s, f->base, 5);
                insert(new_file);
                break;
            default:
                break;
        }
        if(close(in)) {
            fprintf(stderr, "Close function failed\n");
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int get_index(char * dir_path, char ** index_path, node ** global_head) {
    //int out;
    // if ((out = open(*index_path, O_WRONLY|O_CREAT|O_TRUNC|O_APPEND, 0777)) < 0) {
    //     fprintf(stderr, "Open function failed\n");
    //     return EXIT_FAILURE;
    // }
    head = NULL;
    if (nftw(dir_path, walk, MAXFD, FTW_PHYS) != 0) {
        fprintf(stderr, "Nftw function failed\n");
        return EXIT_FAILURE;
    }
    *global_head = head;
    return EXIT_SUCCESS;
}