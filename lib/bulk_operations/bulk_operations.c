#define _GNU_SOURCE
#include <unistd.h>
#include "bulk_operations.h"
#include <errno.h>

ssize_t bulk_read(int fd, void *buf, size_t bytes_to_read){
        return read(fd, buf, bytes_to_read);
        ssize_t c;
        ssize_t bytes_read = 0;
        do {
                c = TEMP_FAILURE_RETRY(read(fd, buf, bytes_to_read));
                if(c < 0) return c;
                if(c == 0) return bytes_read;
                buf += c;
                bytes_read += c;
                bytes_to_read -= c;
        } while(bytes_to_read > 0);
        return bytes_read ;
}

ssize_t bulk_write(int fd, void *buf, size_t bytes_to_write){
        return write(fd, buf, bytes_to_write);
        ssize_t c;
        ssize_t bytes_written = 0;
        do {
                c = TEMP_FAILURE_RETRY(write(fd, buf, bytes_to_write));
                if(c < 0) return c;
                buf += c;
                bytes_written += c;
                bytes_to_write -= c;
        } while(bytes_to_write > 0);
        return bytes_written ;
}


