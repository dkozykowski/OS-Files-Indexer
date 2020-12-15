#include "../../models/node.h"
#define DIR 1
#define JPEG 65496
#define PNG 35152
#define GZIP 8075
#define ZIP 20555


#ifndef _GET_INDEX_H
#define _GET_INDEX_H

int get_index(char * dir_path,  node ** global_head, int * exit_flag);

#endif