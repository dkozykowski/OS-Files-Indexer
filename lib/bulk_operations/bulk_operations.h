#include <stdlib.h>

#ifndef _BULK_OPERATIONS_H
#define _BULK_OPERATIONS_H

ssize_t bulk_read(int fd, void *buf, size_t count);
ssize_t bulk_write(int fd, void *buf, size_t count);

#endif