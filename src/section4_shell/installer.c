/* src/section4_shell/installer.c */

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

// --- Externs from Linker ---
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

// --- Externs from ATA Driver ---
extern uint32_t ide_get_total_sectors();
extern void ide_write_sector_bytes(uint32_t lba, uint8_t* buffer);
extern void ide_read_sector_bytes(uint32_t lba, uint8_t* buffer); // Ensure this exists!

// --- Externs from VGA Driver ---
extern void vga_write(const char* data);
extern void vga_putchar(char c);
extern void vga_set_color(unsigned char color);

#include "grub_payload.h"

void cmd_install_os() {
    vga_set_color(0x0B); 
    vga_write("\n[ AliOS Self-Hosting Installer ]\n");
    vga_set_color(0x0F);

    uint32_t total_sectors = ide_get_total_sectors();
    if (total_sectors == 0) {
        vga_write("Error: No hard drive detected!\n");
        return;
    }

    // --- 1. Write GRUB Stage 1 (Sector 0) Surgically ---
    vga_write("Installing GRUB MBR... ");
    
    uint8_t sector_0[512];
    // Read the existing sector 0 to preserve the partition table
    ide_read_sector_bytes(0, sector_0);

    // Copy only the bootloader code (first 440 bytes)
    // This leaves the partition table (446-510) untouched!
    for (int i = 0; i < 440; i++) {
        sector_0[i] = boot_img[i];
    }

    // Ensure the MBR boot signature is valid
    sector_0[510] = 0x55;
    sector_0[511] = 0xAA;

    ide_write_sector_bytes(0, sector_0);
    vga_write("DONE\n");

    // --- 2. Write GRUB Core (Starting at Sector 1) ---
    vga_write("Writing GRUB Core... ");
    uint32_t core_sectors = (grub_core_img_len + 511) / 512;
    for (uint32_t i = 0; i < core_sectors; i++) {
        ide_write_sector_bytes(1 + i, &grub_core_img[i * 512]);
    }
    vga_write("DONE\n");

    // --- 3. Write AliOS Kernel (Starting at Sector 2048) ---
    vga_write("Writing AliOS Kernel (Sector 2048)... ");
    
    // Starting at 0x1000 where the Multiboot header is mapped
    uint8_t* kernel_ptr = (uint8_t*)0x1000; 
    uint32_t kernel_size = (uint32_t)&_kernel_end - 0x1000;
    uint32_t kernel_sectors = (kernel_size + 511) / 512;

    for (uint32_t i = 0; i < kernel_sectors; i++) {
        ide_write_sector_bytes(2048 + i, kernel_ptr + (i * 512));
        
        // Progress indicator
        if (i % 50 == 0) vga_putchar('.');
    }
    vga_write(" DONE\n");

    vga_set_color(0x0A);
    vga_write("\nSUCCESS! AliOS 4.0 is now installed.\n");
    vga_write("Remove the ISO and reboot from the disk.\n");
    vga_set_color(0x1E);
}
