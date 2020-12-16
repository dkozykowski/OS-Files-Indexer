#include "../models/node.h"

#ifndef _UTILS_H
#define _UTILS_H

#define FATAL(source) (fprintf(stderr, source " function failed\n"),\
            fprintf(stderr, "%s:%d\n", __FILE__, __LINE__),\
            exit(EXIT_FAILURE))

#define ERR(source) fprintf(stderr, source "\n")

#define ELAPSED(start,end) ((end).tv_sec-(start).tv_sec)

void free_old_list(node * head);

#endif