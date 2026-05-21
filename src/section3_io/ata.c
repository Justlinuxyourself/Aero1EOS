#include <stdint.h>
#include <stdbool.h>
// Standard Primary ATA Bus IO Ports
#define ATA_REG_DATA       0x1F0
#define ATA_REG_FEATURES   0x1F1
#define ATA_REG_SEC_COUNT  0x1F2
#define ATA_REG_LBA_LOW    0x1F3
#define ATA_REG_LBA_MID    0x1F4
#define ATA_REG_LBA_HIGH   0x1F5
#define ATA_REG_DRV_SEL    0x1F6
#define ATA_REG_COMMAND    0x1F7
#define ATA_REG_STATUS     0x1F7

// ATA Status Register Bits
#define ATA_STATUS_ERR     0x01  // Error occurred
#define ATA_STATUS_DRQ     0x08  // Data Request (Ready to transfer data)
#define ATA_STATUS_SRV     0x10  // Service request
#define ATA_STATUS_DF      0x20  // Drive Fault
#define ATA_STATUS_RDY     0x40  // Drive Ready
#define ATA_STATUS_BSY     0x80  // Drive Busy

// ATA Commands
#define ATA_CMD_READ       0x20
#define ATA_CMD_WRITE      0x30
#define ATA_CMD_CACHE_FLUSH 0xE7
#define ATA_CMD_IDENTIFY   0xEC

// Low-level Inline Assembly Wrapper functions for IO Ports
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/**
 * 400ns Hardware Delay Loop
 * Reading the Status Register alternate port or primary port 4 times 
 * forces the bus to naturally create an intentional execution buffer delay.
 */
static void ata_io_delay(void) {
    inb(ATA_REG_STATUS);
    inb(ATA_REG_STATUS);
    inb(ATA_REG_STATUS);
    inb(ATA_REG_STATUS);
}

/**
 * Poll the drive status until it drops the BSY bit and clears errors.
 * Returns 0 on success, -1 if a drive fault/error is caught.
 */
static int ata_wait_ready(void) {
    while (1) {
        uint8_t status = inb(ATA_REG_STATUS);
        
        // If the drive is no longer busy, evaluate flag states
        if (!(status & ATA_STATUS_BSY)) {
            if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) {
                return -1; // Device reporting active fault state
            }
            return 0; // Device is completely clear
        }
    }
}

/**
 * Explicit Handshake Loop required specifically before pushing data words
 * to ensure the inner hardware buffer is configured to consume values.
 */
static int ata_wait_drq(void) {
    while (1) {
        uint8_t status = inb(ATA_REG_STATUS);
        
        if (!(status & ATA_STATUS_BSY)) {
            if (status & (ATA_STATUS_ERR | ATA_STATUS_DF)) {
                return -1;
            }
            if (status & ATA_STATUS_DRQ) {
                return 0; // Data Request line is active! Ready to read/write!
            }
        }
    }
}

/**
 * Read 512 Bytes from a targeted disk sector
 */
int ide_read_sector_bytes(uint32_t lba, uint8_t* buffer) {
    if (ata_wait_ready() < 0) return -1;

    // Send drive select, geometry parameters, and upper 4 bits of LBA
    outb(ATA_REG_DRV_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_delay();
    
    // Send standard sector size count parameters (1 Sector = 512 bytes)
    outb(ATA_REG_SEC_COUNT, 1);
    
    // Disperse remaining LBA segments across target out-ports
    outb(ATA_REG_LBA_LOW,  (uint8_t)lba);
    outb(ATA_REG_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_REG_LBA_HIGH, (uint8_t)(lba >> 16));
    
    // Issue Read Command
    outb(ATA_REG_COMMAND, ATA_CMD_READ);
    
    // WAIT for the drive to process the read request and fill its SRAM buffer
    if (ata_wait_drq() < 0) return -1;
    
    // Read 256 Words (512 Bytes) from the hardware data port
    uint16_t* word_buf = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        word_buf[i] = inw(ATA_REG_DATA);
    }
    
    return 0;
}

/**
 * Write 512 Bytes cleanly to a targeted disk sector
 */
int ide_write_sector_bytes(uint32_t lba, uint8_t* buffer) {
    if (ata_wait_ready() < 0) return -1;

    // Send drive select and upper LBA bits
    outb(ATA_REG_DRV_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_delay();
    
    // Set Sector Count to 1
    outb(ATA_REG_SEC_COUNT, 1);
    
    // Disperse LBA segments
    outb(ATA_REG_LBA_LOW,  (uint8_t)lba);
    outb(ATA_REG_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_REG_LBA_HIGH, (uint8_t)(lba >> 16));
    
    // Issue Write Command
    outb(ATA_REG_COMMAND, ATA_CMD_WRITE);
    
    // CRITICAL HANDSHAKE: Wait for the controller to assert DRQ before streaming!
    // This stops the drive from dropping our data into a closed port void!
    if (ata_wait_drq() < 0) return -1;
    
    // Push 256 Words (512 Bytes) to the hardware data port
    uint16_t* word_buf = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_REG_DATA, word_buf[i]);
    }
    
    // Enforce 400ns line clear delay following data streaming burst
    ata_io_delay();
    
    // FLUSH CACHE: Forces QEMU/VirtualBox to commit the virtual buffer data 
    // down into the actual host filesystem .qcow2 or raw image block container.
    outb(ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    if (ata_wait_ready() < 0) return -1;

    return 0;
}
/**
 * Dynamically queries the primary ATA drive using the IDENTIFY command
 * to extract the absolute total number of addressable LBA sectors.
 */
uint32_t ide_get_total_sectors(void) {
    // Wait for drive to be clear to receive commands
    if (ata_wait_ready() < 0) {
        return 0; 
    }

    // Select the primary master drive
    outb(ATA_REG_DRV_SEL, 0xA0);
    ata_io_delay();

    // Sector count and LBA registers must be zeroed for the IDENTIFY command
    outb(ATA_REG_SEC_COUNT, 0);
    outb(ATA_REG_LBA_LOW, 0);
    outb(ATA_REG_LBA_MID, 0);
    outb(ATA_REG_LBA_HIGH, 0);

    // Issue the IDENTIFY command
    outb(ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_delay();

    // Check if the drive is alive and pulling data into its SRAM cache
    uint8_t status = inb(ATA_REG_STATUS);
    if (status == 0) {
        return 0; // No drive connected to the primary bus
    }

    // Wait for the drive buffer to open up for a read transmission burst
    if (ata_wait_drq() < 0) {
        return 0;
    }

    // Read the 256 words (512 bytes) of identification data configuration matrix
    uint16_t identify_data[256];
    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(ATA_REG_DATA);
    }

    // ATA String Specification: Words 60 and 61 contain the total number of 28-bit LBA sectors
    uint32_t total_sectors = *((uint32_t*)&identify_data[60]);

    if (total_sectors == 0) {
        // Fallback check: If LBA28 is 0, check if it's an older CHS-only geometry drive
        uint32_t cylinders = identify_data[1];
        uint32_t heads = identify_data[3];
        uint32_t sectors_per_track = identify_data[6];
        total_sectors = cylinders * heads * sectors_per_track;
    }

    return total_sectors;
}