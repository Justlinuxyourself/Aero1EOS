#ifndef PCI_H
#define PCI_H

#include <stdint.h>

/**
 * Reads a 16-bit word from the PCI configuration space.
 * @param bus    The PCI bus number (0-255)
 * @param slot   The PCI slot number (0-31)
 * @param func   The function number (usually 0)
 * @param offset The register offset (should be 4-byte aligned)
 */
uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

/**
 * Helper: Gets the Vendor ID (Offset 0x00)
 */
uint16_t pci_get_vendor_id(uint8_t bus, uint8_t slot);

/**
 * Helper: Gets the Device ID (Offset 0x02)
 */
uint16_t pci_get_device_id(uint8_t bus, uint8_t slot);

/**
 * Scans the entire PCI bus and prints found devices to VGA.
 */
void pci_scan(void);

/**
 * Helper: Prints a 16-bit value in Hex format to the current TTY.
 */
void vga_write_hex(uint16_t val);

#endif // PCI_H