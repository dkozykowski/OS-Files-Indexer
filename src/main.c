#include <stdio.h>
#include <stdlib.h>
#include "../lib/read_params/read_params.h"


int main(int argc, char ** argv) {
    int n;
    char * index_path, * dir_path;
    index_path = dir_path = NULL;
    if(read_params(argc, argv, &dir_path, &index_path, &n) != 0) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}