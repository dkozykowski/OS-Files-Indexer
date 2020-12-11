#include <stdio.h>
#include <stdlib.h>
#include "../lib/read_params/read_params.h"
#include "../lib/get_index/get_index.h"
#include "../models/node.h"
#include "../models/file_data.h"


int main(int argc, char ** argv) {
    int n;
    char * index_path, * dir_path;
    index_path = dir_path = NULL;
    if(read_params(argc, argv, &dir_path, &index_path, &n) != 0) return EXIT_FAILURE;
    node * head, *temp;
    if(get_index(dir_path, &index_path, &head) != 0) return EXIT_FAILURE;

    temp = head;
    while(temp) {
        printf("Mamy plik %s\n", temp->elem.name);
        temp = temp->next;
    }

    return EXIT_SUCCESS;
}