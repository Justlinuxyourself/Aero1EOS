#include "alifs.h"
#include <stdint.h>
extern void ide_write_sector_bytes(uint32_t lba, uint8_t* buffer);
extern void ide_read_sector_bytes(uint32_t lba, uint8_t* buffer);
extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);
extern int strlen(const char* s);
extern int strncmp(const char* s1, const char* s2, int n);
extern void vga_write(const char* str);

extern char current_path[256]; 

// Static buffer to hold the Inode Table during operations
static uint8_t inode_sector[512] __attribute__((aligned(8)));
// Static buffer for reading file content
static char file_content_buffer[512] __attribute__((aligned(8)));

void alifs_format() {
    vga_write("Formatting AliFS...\n");
    for (int i = 0; i < 512; i++) inode_sector[i] = 0;
    
    // Write empty Inode Table to LBA 20001 (Table Location)
    ide_write_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    vga_write("AliFS Initialized at LBA 20000.\n");
}


int alifs_create(char* name, char* data) {
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    char full_path[FILENAME_LEN];
    // Force absolute paths: "/filename" or "/dir/filename"
    if (strcmp(current_path, "/") == 0) {
        full_path[0] = '/';
        strcpy(full_path + 1, name);
    } else {
        strcpy(full_path, current_path);
        int len = strlen(full_path);
        full_path[len] = '/';
        strcpy(full_path + len + 1, name);
    }

    int slot = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        // STRICT: Only match if the full path is identical
        if (inodes[i].active && strcmp(inodes[i].filename, full_path) == 0) {
            slot = i; break;
        }
        if (slot == -1 && !inodes[i].active) slot = i;
    }

    if (slot == -1) return -1;

    strcpy(inodes[slot].filename, full_path);
    inodes[slot].start_lba = ALIFS_START_LBA + 2 + slot;
    inodes[slot].size = strlen(data);
    inodes[slot].active = 1;
    inodes[slot].is_dir = 0;

    uint8_t write_bounce[512] = {0};
    int data_len = strlen(data) > 511 ? 511 : strlen(data);
    for (int b = 0; b < data_len; b++) write_bounce[b] = (uint8_t)data[b];
    
    ide_write_sector_bytes(inodes[slot].start_lba, write_bounce);
    ide_write_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    return 0;
}


void alifs_list() {
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    vga_write("\nListing: "); vga_write(current_path); vga_write("\n");
    
    int path_len = strlen(current_path);
    int count = 0;

    for (int i = 0; i < MAX_FILES; i++) {
        if (!inodes[i].active) continue;

        // Must start with current path
        if (strncmp(inodes[i].filename, current_path, path_len) == 0) {
            char* remainder = inodes[i].filename + path_len;
            
            // Logic: Is it a direct child?
            // Root "/" -> remainder[0] is not '/', so children are just "file"
            // Subdir "/dir" -> remainder[0] must be '/', remainder[1+] must NOT have '/'
            if (strcmp(current_path, "/") == 0) {
                if (remainder[0] != '/') { // Not a sub-path
                    vga_write(inodes[i].is_dir ? "<DIR> " : "      ");
                    vga_write(remainder); vga_write("\n");
                    count++;
                }
            } else if (remainder[0] == '/' && strchr(remainder + 1, '/') == NULL) {
                vga_write(inodes[i].is_dir ? "<DIR> " : "      ");
                vga_write(remainder + 1); vga_write("\n");
                count++;
            }
        }
    }
    if (count == 0) vga_write("(Empty)\n");
}


char* alifs_read(char* name) {
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    char full_path[FILENAME_LEN];
    if (strcmp(current_path, "/") == 0) {
        full_path[0] = '/';
        strcpy(full_path + 1, name);
    } else {
        strcpy(full_path, current_path);
        int len = strlen(full_path);
        full_path[len] = '/';
        strcpy(full_path + len + 1, name);
    }

    for (int i = 0; i < MAX_FILES; i++) {
        // Only allow match if the full path is exactly correct
        if (inodes[i].active && strcmp(inodes[i].filename, full_path) == 0) {
            ide_read_sector_bytes(inodes[i].start_lba, (uint8_t*)file_content_buffer);
            return file_content_buffer;
        }
    }
    return 0;
}

int alifs_mkdir(char* name) {
    // 1. Load the Inode Table into our buffer
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    int slot = -1;

    // 2. Scan the table for duplicates and find an empty slot
    for (int i = 0; i < MAX_FILES; i++) {
        // Safety: If a file/dir with this name already exists, abort
        if (inodes[i].active && strcmp(inodes[i].filename, name) == 0) {
            vga_write("Error: Directory or file already exists.\n");
            return -1;
        }

        // Find the first available inactive slot
        if (slot == -1 && !inodes[i].active) {
            slot = i;
        }
    }

    // 3. Error if no slots are available
    if (slot == -1) {
        vga_write("Error: Inode table full. Cannot create directory.\n");
        return -1;
    }

    // 4. Fill the Inode metadata
    strcpy(inodes[slot].filename, name);
    inodes[slot].start_lba = 0; // Directories don't store data in sectors yet
    inodes[slot].size = 0;
    inodes[slot].active = 1;
    inodes[slot].is_dir = 1;    // The magic flag that identifies this as a folder

    // 5. Commit the updated table back to the disk at LBA 20001
    ide_write_sector_bytes(ALIFS_START_LBA + 1, inode_sector);

    vga_write("Directory created successfully.\n");
    return 0;
}
int alifs_is_directory(char* name) {
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;
    for (int i = 0; i < MAX_FILES; i++) {
        if (inodes[i].active && strcmp(inodes[i].filename, name) == 0) {
            return inodes[i].is_dir;
        }
    }
    return 0;
}
void alifs_read_into_buffer(char* name, uint8_t* target) {
    extern uint8_t inode_sector[512];
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    for (int i = 0; i < MAX_FILES; i++) {
        if (inodes[i].active && strcmp(inodes[i].filename, name) == 0) {
            ide_read_sector_bytes(inodes[i].start_lba, target);
            return;
        }
    }
}

