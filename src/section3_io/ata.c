#include <stdint.h>
#include <stdbool.h>

// Global variable for the selected base port
uint16_t ata_base_port = 0x1F0;

// Port offsets
#define ATA_REG_DATA       (ata_base_port + 0)
#define ATA_REG_FEATURES   (ata_base_port + 1)
#define ATA_REG_SEC_COUNT  (ata_base_port + 2)
#define ATA_REG_LBA_LOW    (ata_base_port + 3)
#define ATA_REG_LBA_MID    (ata_base_port + 4)
#define ATA_REG_LBA_HIGH   (ata_base_port + 5)
#define ATA_REG_DRV_SEL    (ata_base_port + 6)
#define ATA_REG_COMMAND    (ata_base_port + 7)
#define ATA_REG_STATUS     (ata_base_port + 7)

#define ATA_STATUS_ERR     0x01
#define ATA_STATUS_DRQ     0x08
#define ATA_STATUS_DF      0x20
#define ATA_STATUS_BSY     0x80

#define ATA_CMD_READ       0x20
#define ATA_CMD_WRITE      0x30
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_IDENTIFY   0xEC

#define ATA_TIMEOUT        10000000

static inline void outb(uint16_t port, uint8_t val) { __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint8_t inb(uint16_t port) { uint8_t ret; __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port)); return ret; }
static inline void outw(uint16_t port, uint16_t val) { __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port)); }
static inline uint16_t inw(uint16_t port) { uint16_t ret; __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port)); return ret; }

static void ata_io_delay(void) {
    for(int i = 0; i < 4; i++) inb(ATA_REG_STATUS);
}

// Automatically detects which bus the disk is on
bool ata_probe(uint16_t port) {
    ata_base_port = port;
    if (inb(ATA_REG_STATUS) == 0xFF) return false;
    outb(ATA_REG_DRV_SEL, 0xA0);
    ata_io_delay();
    outb(ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_delay();
    return (inb(ATA_REG_STATUS) != 0);
}

static int ata_wait_ready(void) {
    for (uint32_t i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t status = inb(ATA_REG_STATUS);
        if (!(status & ATA_STATUS_BSY)) {
            if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) return -1;
            return 0;
        }
    }
    return -1;
}

static int ata_wait_drq(void) {
    for (uint32_t i = 0; i < ATA_TIMEOUT; i++) {
        uint8_t status = inb(ATA_REG_STATUS);
        if (!(status & ATA_STATUS_BSY)) {
            if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) return -1;
            if (status & ATA_STATUS_DRQ) return 0;
        }
    }
    return -1;
}

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
