/* src/section5_posix/vfs.c */
#include "vfs.h"

extern void vga_putchar(char c);
extern char wait_for_key(void);
extern char* alifs_read(char* name);
extern void* kmalloc(uint64_t size);
extern int strlen(const char* s);
extern void strcpy(char* dest, const char* src);

typedef struct {
    int used;
    const char* filename;
    char* data_ptr;     
    uint64_t offset;    
    uint64_t size;      
} posix_fd_t;

#define MAX_OPEN_FILES 16
static posix_fd_t file_table[MAX_OPEN_FILES];

void posix_vfs_init(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        file_table[i].used = 0;
        file_table[i].data_ptr = 0;
    }
    file_table[0].used = 1; file_table[0].filename = "/dev/stdin";
    file_table[1].used = 1; file_table[1].filename = "/dev/stdout";
    file_table[2].used = 1; file_table[2].filename = "/dev/stderr";
}

int posix_open(const char* filename) {
    char* shared_buffer = alifs_read((char*)filename);
    if (!shared_buffer) return -1;

    int assigned_fd = -1;
    for (int i = 3; i < MAX_OPEN_FILES; i++) {
        if (!file_table[i].used) {
            assigned_fd = i;
            break;
        }
    }
    if (assigned_fd == -1) return -1;

    uint64_t file_size = strlen(shared_buffer);
    char* private_storage = (char*)kmalloc(file_size + 1);
    if (!private_storage) return -1;

    strcpy(private_storage, shared_buffer);

    file_table[assigned_fd].used = 1;
    file_table[assigned_fd].filename = filename;
    file_table[assigned_fd].data_ptr = private_storage;
    file_table[assigned_fd].offset = 0;
    file_table[assigned_fd].size = file_size;

    return assigned_fd; 
}

int posix_close(int fd) {
    if (fd < 3 || fd >= MAX_OPEN_FILES || !file_table[fd].used) return -1;
    file_table[fd].used = 0;
    file_table[fd].filename = 0;
    file_table[fd].data_ptr = 0;
    file_table[fd].offset = 0;
    file_table[fd].size = 0;
    return 0;
}

int posix_write(int fd, const char* buf, uint64_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].used || !buf) return -1;
    if (fd == 1 || fd == 2) {
        for (uint64_t i = 0; i < count; i++) vga_putchar(buf[i]);
        return count;
    }
    return -1; 
}

int posix_read(int fd, char* buf, uint64_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].used || !buf) return -1;

    if (fd == 0) {
        uint64_t read_bytes = 0;
        while (read_bytes < count) {
            char c = wait_for_key();
            if (c == 0) continue;
            buf[read_bytes++] = c;
            if (c == '\n') break;
        }
        return read_bytes;
    }

    if (fd >= 3 && file_table[fd].data_ptr != 0) {
        posix_fd_t* file = &file_table[fd];
        uint64_t available = file->size - file->offset;
        if (available == 0) return 0; // EOF
        
        uint64_t to_read = (count > available) ? available : count;
        for (uint64_t i = 0; i < to_read; i++) {
            buf[i] = file->data_ptr[file->offset + i];
        }
        file->offset += to_read;
        return to_read;
    }
    return -1;
}