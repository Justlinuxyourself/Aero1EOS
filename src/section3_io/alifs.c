#include "alifs.h"


extern void ide_write_sector_bytes(uint32_t lba, void* buffer);
extern void ide_read_sector_bytes(uint32_t lba, void* buffer);

extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);
extern int strlen(const char* s);

extern void vga_write(const char* str);


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
    // 1. Read the Inode Table
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    int slot = -1;

    // 2. Check if file exists (overwrite) or find empty slot
    for (int i = 0; i < MAX_FILES; i++) {
        if (inodes[i].active && strcmp(inodes[i].filename, name) == 0) {
            slot = i;
            break;
        }
        if (slot == -1 && !inodes[i].active) {
            slot = i;
        }
    }

    if (slot == -1) return -1; // No space left

    // 3. Setup Inode metadata
    // Each file gets its own sector starting at LBA + 2 + slot
    strcpy(inodes[slot].filename, name);
    inodes[slot].start_lba = ALIFS_START_LBA + 2 + slot;
    inodes[slot].size = strlen(data);
    inodes[slot].active = 1;

    // 4. Write data to the assigned data sector
    ide_write_sector_bytes(inodes[slot].start_lba, (uint8_t*)data);

    // 5. Update the Inode Table on disk
    ide_write_sector_bytes(ALIFS_START_LBA + 1, inode_sector);

    return 0;
}

char* alifs_read(char* name) {
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    for (int i = 0; i < MAX_FILES; i++) {
        if (inodes[i].active && strcmp(inodes[i].filename, name) == 0) {
            // Clear buffer and read the data sector
            for(int j=0; j<512; j++) file_content_buffer[j] = 0;
            ide_read_sector_bytes(inodes[i].start_lba, (uint8_t*)file_content_buffer);
            return file_content_buffer;
        }
    }
    return 0; // Not found
}

void alifs_list() {
    ide_read_sector_bytes(ALIFS_START_LBA + 1, inode_sector);
    alifs_inode_t* inodes = (alifs_inode_t*)inode_sector;

    vga_write("\n--- AliFS File Listing ---\n");
    int count = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (inodes[i].active) {
            vga_write("- ");
            vga_write(inodes[i].filename);
            count++;
        }
    }
    if (count == 0) vga_write("(Empty)\n");
}
