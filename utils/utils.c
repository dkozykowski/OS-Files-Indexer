#include "utils.h"
#include "../models/node.h"
#include <stdlib.h>

void free_old_list(node * head) {
    node * temp;
    while(head) {
        temp = head;
        head = head->next;
        if(temp->elem.name) free(temp->elem.name);
        if (temp->elem.path) free(temp->elem.path);
        free(temp);
    }
}