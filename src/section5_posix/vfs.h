/* src/section5_posix/vfs.h */
#ifndef POSIX_VFS_H
#define POSIX_VFS_H

#include <stdint.h>

// Initialize File Descriptors 0, 1, and 2
void posix_vfs_init(void);

// Core POSIX System Functions
int  posix_open(const char* filename);
int  posix_close(int fd);
int  posix_read(int fd, char* buf, uint64_t count);
int  posix_write(int fd, const char* buf, uint64_t count);

#endif