#include "../section1_cpu/io.h"
#include <stdint.h>

/* ATA Status Bits */
#define STATUS_BSY  0x80
#define STATUS_RDY  0x40
#define STATUS_DF   0x20
#define STATUS_DRQ  0x08
#define STATUS_ERR  0x01

/* ATA Ports */
#define ATA_PRIMARY_DATA         0x1F0
#define ATA_PRIMARY_ERR_FEATURES 0x1F1
#define ATA_PRIMARY_SEC_COUNT    0x1F2
#define ATA_PRIMARY_LBA_LOW      0x1F3
#define ATA_PRIMARY_LBA_MID      0x1F4
#define ATA_PRIMARY_LBA_HIGH     0x1F5
#define ATA_PRIMARY_DRIVE_SEL    0x1F6
#define ATA_PRIMARY_COMMAND      0x1F7
#define ATA_PRIMARY_CONTROL      0x3F6

extern uint16_t inw(uint16_t port);
extern void outw(uint16_t port, uint16_t data);

/* 
 * 400ns delay required by the ATA spec after drive switching/commands.
 * Reading the Control Register 4 times is the standard way to achieve this.
 */
static void ide_io_wait() {
    inb(ATA_PRIMARY_CONTROL);
    inb(ATA_PRIMARY_CONTROL);
    inb(ATA_PRIMARY_CONTROL);
    inb(ATA_PRIMARY_CONTROL);
}

/*
 * Advanced polling: Checks for BSY, RDY, and ERR.
 * Returns 0 on success, -1 on hardware error or missing drive.
 */
static int ide_poll() {
    while (1) {
        uint8_t status = inb(ATA_PRIMARY_COMMAND);

        // Check for floating bus (no drive connected)
        if (status == 0xFF) return -1;

        // Check for Drive Fault or Error bits
        if (status & (STATUS_ERR | STATUS_DF)) return -1;

        // Drive is only ready if BSY is 0 and RDY is 1
        if (!(status & STATUS_BSY) && (status & STATUS_RDY)) {
            return 0;
        }
    }
}

static void ide_wait_drq() {
    while (!(inb(ATA_PRIMARY_COMMAND) & STATUS_DRQ));
}

void ide_read_sector_bytes(uint32_t lba, uint8_t* buffer) {
    if (ide_poll() < 0) return;

    // Select Drive (Master) and LBA bits 24-27
    outb(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    ide_io_wait();

    outb(ATA_PRIMARY_SEC_COUNT, 1);
    outb(ATA_PRIMARY_LBA_LOW,  (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND,  0x20); // READ SECTORS

    ide_io_wait();

    // Wait for the drive to process the command and set DRQ
    if (ide_poll() < 0) return;
    ide_wait_drq();

    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(ATA_PRIMARY_DATA);
        buffer[i * 2] = (uint8_t)data;
        buffer[i * 2 + 1] = (uint8_t)(data >> 8);
    }
}

void ide_write_sector_bytes(uint32_t lba, uint8_t* buffer) {
    if (ide_poll() < 0) return;

    outb(ATA_PRIMARY_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    ide_io_wait();

    outb(ATA_PRIMARY_SEC_COUNT, 1);
    outb(ATA_PRIMARY_LBA_LOW,  (uint8_t)lba);
    outb(ATA_PRIMARY_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_PRIMARY_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_PRIMARY_COMMAND,  0x30); // WRITE SECTORS

    ide_io_wait();

    if (ide_poll() < 0) return;
    ide_wait_drq();

    for (int i = 0; i < 256; i++) {
        uint16_t data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(ATA_PRIMARY_DATA, data);
    }

    // Flush cache to ensure data is written to the platter
    ide_io_wait();
    outb(ATA_PRIMARY_COMMAND, 0xE7); // CACHE FLUSH
    ide_poll();
}

uint32_t ide_get_total_sectors() {
    if (ide_poll() < 0) return 0;

    outb(ATA_PRIMARY_DRIVE_SEL, 0xA0); // Select Master
    ide_io_wait();
    outb(ATA_PRIMARY_COMMAND, 0xEC);  // IDENTIFY

    if (ide_poll() < 0) return 0;
    ide_wait_drq();

    uint16_t info[256];
    for (int i = 0; i < 256; i++) {
        info[i] = inw(ATA_PRIMARY_DATA);
    }

    // LBA28 total sectors at words 60 and 61
    return (uint32_t)info[60] | ((uint32_t)info[61] << 16);
}

uint32_t ide_calculate_pseudo_used_sectors(uint32_t max_sectors_to_scan) {
    uint8_t buffer[512];
    uint32_t used = 0;

    for (uint32_t s = 0; s < max_sectors_to_scan; s++) {
        // Clear buffer to prevent false positives from previous reads
        for (int b = 0; b < 512; b++) buffer[b] = 0;

        ide_read_sector_bytes(s, buffer);

        for (int i = 0; i < 512; i++) {
            if (buffer[i] != 0) {
                used++;
                break; 
            }
        }
    }
    return used;
}
