#include "grub_payload.h"

/* --- Types --- */
typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

/* --- Externs from your Drivers --- */
extern void vga_write(const char* data);
extern void vga_putchar(char c);
extern void vga_set_color(unsigned char color);
extern void ide_read_sector_bytes(uint32_t lba, uint8_t* buffer);
extern void ide_write_sector_bytes(uint32_t lba, uint8_t* buffer);
extern void cmd_disk_wipe();
/**
 * itoa
 * Converts an integer to a string for display in AeroOS.
 */
static void itoa(uint32_t n, char* s) {
    uint32_t i = 0, j;
    char tmp;
    if (n == 0) s[i++] = '0';
    while (n > 0) {
        s[i++] = (n % 10) + '0';
        n /= 10;
    }
    s[i] = '\0';
    
    // Reverse the string to get digits in correct order
    for (j = 0; j < i / 2; j++) {
        tmp = s[j];
        s[j] = s[i - j - 1];
        s[i - j - 1] = tmp;
    }
}

void cmd_install_os() {
    vga_set_color(0x0B); // Cyan
    vga_write("\n[ AeroOS 4.0 Self-Hosting Installer ]\n");
    vga_set_color(0x0F); // White

    // --- 1. BUILD SYSTEM CHECK ---
    if (alios4_bin_len <= 1) {
        vga_set_color(0x0C); // Red
        vga_write("CRITICAL ERROR: Kernel payload is empty (1 byte)!\n");
        vga_write("Fix: Run 'make clean' and then 'make' twice.\n");
        return;
    }
    // --- 2. WIPE DISK --- 
    cmd_disk_wipe();
    // --- 3. INSTALL GRUB MBR (SECTOR 0) ---
    vga_write("Step 1: Installing GRUB MBR... ");
    uint8_t sector_0[512];
    
    // FIX: Wipe the buffer completely clean first to remove real hardware garbage
    for (int i = 0; i < 512; i++) {
        sector_0[i] = 0x00;
    }

    // FIX: Copy all 512 bytes of GRUB's boot.img instead of truncating at 440
    // This includes GRUB's internal jump offsets and safe empty structures
    for (int i = 0; i < 512; i++) {
        sector_0[i] = boot_img[i];
    }

    // FIX: Inject a guaranteed active partition entry so the physical BIOS doesn't skip it
    sector_0[446] = 0x80; // Bootable flag (Active)
    sector_0[447] = 0x01; // Starting Head
    sector_0[448] = 0x01; // Starting Sector
    sector_0[449] = 0x00; // Starting Cylinder
    sector_0[450] = 0x83; // Linux/Raw filesystem type
    sector_0[451] = 0xFE; // Ending Head
    sector_0[452] = 0xFF; // Ending Sector
    sector_0[453] = 0xFF; // Ending Cylinder
    
    // Starting LBA Sector (Sector 2048, where AeroOS lives)
    sector_0[454] = 0x00; 
    sector_0[455] = 0x08; 
    sector_0[456] = 0x00; 
    sector_0[457] = 0x00;

    // Force the standard boot signature explicitly just in case
    sector_0[510] = 0x55;
    sector_0[511] = 0xAA;

    ide_write_sector_bytes(0, sector_0);
    vga_write("DONE\n");

    // --- 4. INSTALL GRUB CORE (SECTOR 1+) ---
    vga_write("Step 2: Writing GRUB Core... ");
    uint32_t core_sectors = (grub_core_img_len + 511) / 512;
    for (uint32_t i = 0; i < core_sectors; i++) {
        ide_write_sector_bytes(1 + i, &grub_core_img[i * 512]);
    }
    vga_write("DONE\n");

    // --- 5. INSTALL ALIOS KERNEL (SECTOR 2048) ---
    vga_write("Step 3: Deploying Kernel to Sector 2048... ");
    uint32_t k_sectors = (alios4_bin_len + 511) / 512;

    for (uint32_t i = 0; i < k_sectors; i++) {
        ide_write_sector_bytes(2048 + i, &alios4_bin[i * 512]);
        if (i % 25 == 0) vga_putchar('.');
    }
    vga_write(" DONE\n");

    // --- 6. SUCCESS SUMMARY ---
    vga_set_color(0x0A); // Green
    vga_write("\nSUCCESS! AeroOS 4.0 is now on (hd0).\n");
    vga_set_color(0x0F); // White
    
    vga_write("Reboot and type this into GRUB:\n");
    vga_set_color(0x0E); // Yellow
    vga_write("multiboot2 (hd0)2048+");
    
    char count_str[16];
    itoa(k_sectors, count_str);
    vga_write(count_str);
    
    vga_write("\nboot\n");
    vga_set_color(0x1E); 
}
