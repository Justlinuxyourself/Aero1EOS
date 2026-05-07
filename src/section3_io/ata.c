#include "../section1_cpu/io.h"
#include <stdint.h>
#define STATUS_BSY 0x80
#define STATUS_RDY 0x40
#define STATUS_DRQ 0x08
#define STATUS_DF 0x20
#define STATUS_ERR 0x01
extern uint16_t inw(uint16_t port);

static void ide_wait_bsy() {
    uint8_t status;
    while (1) {
        status = inb(0x1F7);
        if (status == 0xFF) break; // Floating bus/No drive - avoid infinite loop
        if (!(status & STATUS_BSY)) break;
    }
}

static void ide_wait_drq() {
	while (!(inb(0x1F7) & STATUS_DRQ));
}
static void ide_io_wait() {
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);
}

void ide_read_sector_bytes(uint32_t lba, uint8_t* buffer) {
    ide_wait_bsy();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    ide_io_wait(); // Wait after drive select
    
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20); // READ SECTORS
    
    ide_io_wait(); // CRITICAL: Wait 400ns for the drive to set BSY
    
    ide_wait_bsy();
    ide_wait_drq(); // Wait for Data Request
    
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(0x1F0);
        buffer[i * 2] = (uint8_t)data;
        buffer[i * 2 + 1] = (uint8_t)(data >> 8);
    }
}

void ide_write_sector_bytes(uint32_t lba, uint8_t* buffer) {
	ide_wait_bsy();
	outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
	outb(0x1F2, 1);
	outb(0x1F3, (uint8_t)lba);
	outb(0x1F4, (uint8_t)(lba >> 8));
	outb(0x1F5, (uint8_t)(lba >> 16));
	outb(0x1F7, 0x30);
    ide_io_wait();
	ide_wait_bsy();
	ide_wait_drq();
	for (int i = 0; i < 256; i++) {
		uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
		outw(0x1F0, data);
	}
	outb(0x1F7, 0xE7);
    ide_io_wait();
	ide_wait_bsy();
}

// Returns the total capacity of the drive in sectors
uint32_t ide_get_total_sectors() {
    ide_wait_bsy();
    
    // Select Master Drive (0xA0)
    outb(0x1F6, 0xA0);
    
    // IDENTIFY command
    outb(0x1F7, 0xEC);
    
    ide_wait_bsy();
    
    // Check if drive exists (status 0 means no drive)
    if (inb(0x1F7) == 0) return 0;

    ide_wait_drq();

    // Read the 512-byte identification block
    uint16_t info[256];
    for (int i = 0; i < 256; i++) {
        info[i] = inw(0x1F0);
    }

    // LBA28 Sector count is stored in words 60 and 61
    uint32_t sectors = (uint32_t)info[60] | ((uint32_t)info[61] << 16);
    return sectors;
}

uint32_t ide_calculate_pseudo_used_sectors(uint32_t max_sectors_to_scan) {
    uint8_t buffer[512];
    uint32_t used = 0;

    // We only scan the start of the disk so it doesn't take forever
    for (uint32_t s = 0; s < max_sectors_to_scan; s++) {
        ide_read_sector_bytes(s, buffer);
        for (int i = 0; i < 512; i++) {
            if (buffer[i] != 0) {
                used++;
                break; // Sector is "used"
            }
        }
    }
    return used;
}
