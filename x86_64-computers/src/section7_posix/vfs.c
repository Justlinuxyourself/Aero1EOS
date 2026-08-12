#include "../section8_global-header/global.h"

#include "../section3_io/alifs.h"
#include <string.h>

static file_descriptor_t open_files[16];

int vfs_open(char* path) {
    // Check if file exists in AliFS
    if (alifs_read(path) == 0) return -1;
    
    for (int i = 0; i < 16; i++) {
        if (!open_files[i].active) {
            open_files[i].active = 1;
            strcpy(open_files[i].path, path);
            return i; 
        }
    }
    return -1;
}

int vfs_read(int fd, uint8_t* buffer, int count) {
    if (fd < 0 || fd >= 16 || !open_files[fd].active) return -1;
    alifs_read_into_buffer(open_files[fd].path, buffer);
    return count;
}

void vfs_write(int fd, uint8_t* buffer, int count) {
    if (fd == 1) { // Standard Output
        for(int i = 0; i < count; i++) {
            char t[2] = {(char)buffer[i], 0};
            vga_write(t);
        }
    }
}
