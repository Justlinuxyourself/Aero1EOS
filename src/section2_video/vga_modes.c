#include <stdint.h>

void set_text_mode() {
    volatile uint8_t* ports = (volatile uint8_t*)0x3C0;
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x67), "Nd"((uint16_t)0x3C2));
    uint8_t values[5] = {0x03, 0x00, 0x03, 0x00, 0x02};
    for(int i=0; i<5; i++) {
        __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)i), "Nd"((uint16_t)0x3C4));
        __asm__ volatile ("outb %0, %1" : : "a"(values[i]), "Nd"((uint16_t)0x3C5));
    }
    uint8_t crtc[25] = {0x5F,0x4F,0x50,0x82,0x55,0x81,0xBF,0x1F,0x00,0x4F,0x0D,0x0E,0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x28,0x1F,0x96,0xB9,0xA3,0xFF};
    for(int i=0; i<25; i++) {
        __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)i), "Nd"((uint16_t)0x3D4));
        __asm__ volatile ("outb %0, %1" : : "a"(crtc[i]), "Nd"((uint16_t)0x3D5));
    }
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x05), "Nd"((uint16_t)0x3CE));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x10), "Nd"((uint16_t)0x3CF));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x06), "Nd"((uint16_t)0x3CE));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x02), "Nd"((uint16_t)0x3CF));
    uint8_t dummy = *(ports + 0x1A); (void)dummy;
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)16), "Nd"((uint16_t)0x3C0));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)12), "Nd"((uint16_t)0x3C0));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)32), "Nd"((uint16_t)0x3C0));
}

void set_gui_mode() {
    volatile uint8_t* ports = (volatile uint8_t*)0x3C0;
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x63), "Nd"((uint16_t)0x3C2));
    uint8_t values[5] = {0x03, 0x00, 0x0F, 0x00, 0x0E};
    for(int i=0; i<5; i++) {
        __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)i), "Nd"((uint16_t)0x3C4));
        __asm__ volatile ("outb %0, %1" : : "a"(values[i]), "Nd"((uint16_t)0x3C5));
    }
    uint8_t crtc[25] = {0x5F,0x3F,0x40,0x82,0x4B,0x1A,0xBF,0x1F,0x00,0x41,0x00,0x00,0x00,0x00,0x00,0x00,0x9C,0x8E,0x8F,0x14,0x40,0x96,0xB9,0xA2,0xFF};
    for(int i=0; i<25; i++) {
        __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)i), "Nd"((uint16_t)0x3D4));
        __asm__ volatile ("outb %0, %1" : : "a"(crtc[i]), "Nd"((uint16_t)0x3D5));
    }
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x05), "Nd"((uint16_t)0x3CE));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x40), "Nd"((uint16_t)0x3CF));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x06), "Nd"((uint16_t)0x3CE));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)0x05), "Nd"((uint16_t)0x3CF));
    uint8_t dummy = *(ports + 0x1A); (void)dummy;
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)16), "Nd"((uint16_t)0x3C0));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)65), "Nd"((uint16_t)0x3C0));
    __asm__ volatile ("outb %0, %1" : : "a"((uint8_t)32), "Nd"((uint16_t)0x3C0));
}
