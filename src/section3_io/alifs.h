#ifndef ALIFS_H
#define ALIFS_H

#include <stdint.h>

#define ALIFS_START_LBA 20000  // Safe area on your disk
#define MAX_FILES 16           // 16 files fit in one 512-byte sector
#define FILENAME_LEN 32

typedef struct {
    char filename[FILENAME_LEN];
    uint32_t start_lba;
    uint32_t size;
    uint8_t active;   
    uint8_t is_dir;   // 1 if directory, 0 if file
} __attribute__((packed)) alifs_inode_t;

// Filesystem Functions
void alifs_format();
int  alifs_create(char* name, char* data);
char* alifs_read(char* name);
void alifs_list();
int alifs_mkdir(char* name);

#endif
