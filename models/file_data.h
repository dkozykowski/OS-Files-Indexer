#include <sys/types.h>
#include <unistd.h>

#ifndef _FILE_DATA_H
#define _FILE_DATA_H

typedef struct file {
    char * name;
    char * path;
    ssize_t size;
    uid_t owner_uid;
    unsigned int type;
} file;

#endif