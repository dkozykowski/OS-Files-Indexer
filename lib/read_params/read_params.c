#include "read_params.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define NOT_SET -1

static void usage(char * pname) {
    fprintf(stderr, "USAGE: %s [-d DIR_PATH] [-f INDEX_PATH] [-t TIME]\n", pname);
    fprintf(stderr, "DIR_PATH - path to the directory to be indexed\n");
    fprintf(stderr, "INDEX_PATH - path to the file to store index data\n");
    fprintf(stderr, "TIME - interval time between each directory indexing [30, 7200]\n");
    exit(EXIT_FAILURE);
}

static int read_command_line_arguments(int argc, char ** argv, char ** dir_path, char ** index_path, int * n) {
    int c;
    while((c = getopt(argc, argv, "d:f:t:")) != -1) {
        switch(c) {
            case 'd':
                if (*dir_path) usage(argv[0]);
                (*dir_path) = optarg;
                break;
            case 'f':
                if (*index_path) usage(argv[0]);
                (*index_path) = optarg;
                break;
            case 't':
                if ((*n) != 0) usage(argv[0]);
                (*n) = atoi(optarg);
                if ((*n) < 30 || (*n) > 7200) usage(argv[0]);
                break;
            case '?':
            default: usage(argv[0]);
        }
    }
    if (argc > optind) usage(argv[0]);
    return EXIT_SUCCESS;
}

static int load_default_if_not_set(char ** dir_path, char ** index_path, int * n) {
    if (!(*n)) (*n) = NOT_SET; 
    if (*dir_path == NULL) {
        if (((*dir_path) = getenv("MOLE_DIR")) == NULL) {
            fprintf(stderr, "-d DIR_PATH parameter was not given and MOLE_DIR variable was not set\n");
            return EXIT_FAILURE;
        }
    }
    if (*index_path == NULL) {
        if (((*index_path) = getenv("MOLE_INDEX_PATH")) == NULL) {
            (*index_path) = "/.mole-index";
        }
    }
    return EXIT_SUCCESS;
}

int read_params(int argc, char ** argv, char ** dir_path, char ** index_path, int * n) {
    if (read_command_line_arguments(argc, argv, dir_path, index_path, n) != 0) return EXIT_FAILURE;
    if (load_default_if_not_set(dir_path, index_path, n) != 0) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}