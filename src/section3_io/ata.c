#include <stdint.h>
#include <stddef.h>

/* --- PORT DEFINITIONS --- */
#define ATA_REG_DATA       0x1F0
#define ATA_REG_FEATURES   0x1F1
#define ATA_REG_SEC_COUNT  0x1F2
#define ATA_REG_LBA_LOW    0x1F3
#define ATA_REG_LBA_MID    0x1F4
#define ATA_REG_LBA_HIGH   0x1F5
#define ATA_REG_DRV_SEL    0x1F6
#define ATA_REG_COMMAND    0x1F7
#define ATA_REG_STATUS     0x1F7

/* --- STATUS BITS --- */
#define ATA_STATUS_ERR     0x01
#define ATA_STATUS_DRQ     0x08
#define ATA_STATUS_DF      0x20
#define ATA_STATUS_BSY     0x80

/* --- COMMANDS --- */
#define ATA_CMD_READ       0x20
#define ATA_CMD_WRITE      0x30
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_IDENTIFY   0xEC

/* --- SAFETY TIMEOUT --- */
#define ATA_TIMEOUT        10000000

/* --- LOW-LEVEL I/O --- */
static inline void outb(uint16_t port, uint8_t val) { __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint8_t inb(uint16_t port) { uint8_t ret; __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port)); return ret; }
static inline void outw(uint16_t port, uint16_t val) { __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint16_t inw(uint16_t port) { uint16_t ret; __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port)); return ret; }

static void ata_io_delay(void) {
    inb(ATA_REG_STATUS); inb(ATA_REG_STATUS);
    inb(ATA_REG_STATUS); inb(ATA_REG_STATUS);
}

/* --- POLLING ENGINE --- */
static int ata_wait_ready(void) {
    for (uint32_t i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t status = inb(ATA_REG_STATUS);
        if (!(status & ATA_STATUS_BSY)) {
            if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) return -1;
            return 0;
        }
    }
    return -1; // Timeout
}

static int ata_wait_drq(void) {
    for (uint32_t i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t status = inb(ATA_REG_STATUS);
        if (!(status & ATA_STATUS_BSY)) {
            if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) return -1;
            if (status & ATA_STATUS_DRQ) return 0;
        }
    }
    return -1; // Timeout
}

/* --- PUBLIC API --- */

int ide_read_sector_bytes(uint32_t lba, uint8_t* buffer) {
    if (ata_wait_ready() < 0) return -1;
    outb(ATA_REG_DRV_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_delay();
    outb(ATA_REG_SEC_COUNT, 1);
    outb(ATA_REG_LBA_LOW, (uint8_t)lba);
    outb(ATA_REG_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_REG_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_REG_COMMAND, ATA_CMD_READ);
    if (ata_wait_drq() < 0) return -1;
    
    uint16_t* word_buf = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) word_buf[i] = inw(ATA_REG_DATA);
    return 0;
}

int ide_write_sector_bytes(uint32_t lba, uint8_t* buffer) {
    if (ata_wait_ready() < 0) return -1;
    outb(ATA_REG_DRV_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_delay();
    outb(ATA_REG_SEC_COUNT, 1);
    outb(ATA_REG_LBA_LOW, (uint8_t)lba);
    outb(ATA_REG_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_REG_LBA_HIGH, (uint8_t)(lba >> 16));
    outb(ATA_REG_COMMAND, ATA_CMD_WRITE);
    if (ata_wait_drq() < 0) return -1;
    
    uint16_t* word_buf = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) outw(ATA_REG_DATA, word_buf[i]);
    
    ata_io_delay();
    outb(ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    return ata_wait_ready();
}

uint32_t ide_get_total_sectors(void) {
    if (ata_wait_ready() < 0) return 0;
    outb(ATA_REG_DRV_SEL, 0xA0);
    ata_io_delay();
    outb(ATA_REG_SEC_COUNT, 0);
    outb(ATA_REG_LBA_LOW, 0);
    outb(ATA_REG_LBA_MID, 0);
    outb(ATA_REG_LBA_HIGH, 0);
    outb(ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_delay();
    
    if (inb(ATA_REG_STATUS) == 0 || ata_wait_drq() < 0) return 0;
    
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) identify_data[i] = inw(ATA_REG_DATA);
    
    uint32_t total = *((uint32_t*)&identify_data[60]);
    return (total == 0) ? (uint32_t)(identify_data[1] * identify_data[3] * identify_data[6]) : total;
}
