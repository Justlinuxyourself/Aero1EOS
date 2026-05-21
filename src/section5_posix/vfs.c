/* src/section5_posix/vfs.c */
#include "vfs.h"

extern void vga_putchar(char c);
extern char wait_for_key(void);
extern char* alifs_read(char* name);
extern int alifs_create(char* name, char* data);
extern void* kmalloc(uint64_t size);
extern int strlen(const char* s);
extern void strcpy(char* dest, const char* src);

typedef struct {
    int used;
    char filename[32];  // Keep a safe local copy of the filename
    char* data_ptr;     // Dynamic buffer holding the working text
    uint64_t offset;    // Current read/write head position
    uint64_t size;      // Current size of file content
    uint64_t capacity;  // Maximum allocated heap space for this file descriptor
} posix_fd_t;

#define MAX_OPEN_FILES 16
#define FILE_MAX_CAPACITY 512 // Limit to 1 sector layout for now to match AliFS
static posix_fd_t file_table[MAX_OPEN_FILES];

void posix_vfs_init(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        file_table[i].used = 0;
        file_table[i].data_ptr = 0;
    }
    file_table[0].used = 1; strcpy(file_table[0].filename, "/dev/stdin");
    file_table[1].used = 1; strcpy(file_table[1].filename, "/dev/stdout");
    file_table[2].used = 1; strcpy(file_table[2].filename, "/dev/stderr");
}

int posix_open(const char* filename) {
    int assigned_fd = -1;
    for (int i = 3; i < MAX_OPEN_FILES; i++) {
        if (!file_table[i].used) {
            assigned_fd = i;
            break;
        }
    }
    if (assigned_fd == -1) return -1;

    // Allocate a fixed 512-byte block on the heap for this stream workspace
    char* private_storage = (char*)kmalloc(FILE_MAX_CAPACITY);
    if (!private_storage) return -1;
    for (int i = 0; i < FILE_MAX_CAPACITY; i++) private_storage[i] = 0;

    // Check if the file already exists in AliFS
    char* shared_buffer = alifs_read((char*)filename);
    
    file_table[assigned_fd].used = 1;
    strcpy(file_table[assigned_fd].filename, filename);
    file_table[assigned_fd].data_ptr = private_storage;
    file_table[assigned_fd].capacity = FILE_MAX_CAPACITY;

    if (shared_buffer) {
        // Existing file: load data and set cursor to start
        strcpy(private_storage, shared_buffer);
        file_table[assigned_fd].size = strlen(shared_buffer);
        file_table[assigned_fd].offset = 0; 
    } else {
        // Brand new file
        file_table[assigned_fd].size = 0;
        file_table[assigned_fd].offset = 0;
    }

    return assigned_fd; 
}

int posix_close(int fd) {
    if (fd < 3 || fd >= MAX_OPEN_FILES || !file_table[fd].used) return -1;

    posix_fd_t* file = &file_table[fd];

    // --- CRITICAL AUTO-SAVE STEP ---
    // Before wiping the workspace, flush the memory tracking structure into AliFS!
    if (file->data_ptr) {
        alifs_create(file->filename, file->data_ptr);
    }

    file->used = 0;
    file->filename[0] = '\0';
    file->data_ptr = 0;
    file->offset = 0;
    file->size = 0;
    file->capacity = 0;
    return 0;
}

int posix_write(int fd, const char* buf, uint64_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].used || !buf) return -1;

    // Handle Standard Terminal Output
    if (fd == 1 || fd == 2) {
        for (uint64_t i = 0; i < count; i++) vga_putchar(buf[i]);
        return count;
    }

    // Handle Disk File Writing
    if (fd >= 3 && file_table[fd].data_ptr != 0) {
        posix_fd_t* file = &file_table[fd];

        // Ensure we don't break past our 512 byte boundary sector limit
        if (file->offset + count >= file->capacity) {
            count = file->capacity - file->offset - 1; 
        }

        if (count <= 0) return 0;

        // Copy bytes sequentially into our private memory buffer workspace
        for (uint64_t i = 0; i < count; i++) {
            file->data_ptr[file->offset + i] = buf[i];
        }

        file->offset += count;
        
        // If we grew the file, update our string length tracker
        if (file->offset > file->size) {
            file->size = file->offset;
            file->data_ptr[file->size] = '\0'; // Always maintain the null terminator
        }

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