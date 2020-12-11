#include "file_data.h"
#ifndef _NODE_H
#define _NODE_H

typedef struct node {
    file elem;
    struct node * next;
} node;

#endif