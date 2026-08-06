#include "../section1_cpu/io.h"
extern void vga_write();
// PCI ports
#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// Helper to write to PCI Address Port (32-bit)
void pci_write_addr(uint32_t addr) {
    // Write the full 32-bit address at once to the PCI Config Address port
    outl(PCI_CONFIG_ADDR, addr);
}

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000);
    pci_write_addr(address);
    // Read from the data port + offset within the 32-bit register
    return inw(PCI_CONFIG_DATA + (offset & 2));
}

uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot) {
    return pci_read_word(bus, slot, 0, 0x00);
}

uint16_t pci_get_device_id(uint8_t bus, uint8_t slot) {
    return pci_read_word(bus, slot, 0, 0x02);
}

// Simple hex printer for verbose log
void vga_write_hex(uint16_t val) {
    char hex_chars[] = "0123456789ABCDEF";
    char buf[5] = {0};
    buf[3] = hex_chars[val & 0xF];
    buf[2] = hex_chars[(val >> 4) & 0xF];
    buf[1] = hex_chars[(val >> 8) & 0xF];
    buf[0] = hex_chars[(val >> 12) & 0xF];
    vga_write(buf);
}

void pci_scan() {
    vga_write("\n[PCI] Scanning bus...\n");
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint16_t vendor = pci_get_vendor_id((uint8_t)bus, slot);
            
            // 0xFFFF means no device exists at this slot
            if (vendor != 0xFFFF) {
                uint16_t device = pci_get_device_id((uint8_t)bus, slot);
                
                vga_write("  -> Found [");
                vga_write_hex(vendor);
                vga_write(":");
                vga_write_hex(device);
                vga_write("] at Bus ");
                vga_write_hex(bus);
                vga_write(" Slot ");
                vga_write_hex(slot);
                vga_write("\n");
            }
        }
    }
}
