/* Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "tui_video.h"
#include "../section1_cpu/io.h" // For cursor manipulation if needed, maybe will put soon

void tui_clear(uint8_t attribute) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEM_ADDR;
    uint16_t blank = (uint16_t)' ' | ((uint16_t)attribute << 8);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = blank;
    }
}

void tui_print_at(int x, int y, const char* str, uint8_t attribute) {
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEM_ADDR + (y * VGA_WIDTH + x);
    
    while (*str && x < VGA_WIDTH) {
        *vga++ = (uint16_t)*str++ | ((uint16_t)attribute << 8);
        x++;
    }
}

void tui_draw_box(int x, int y, int width, int height, const char* title, uint8_t attribute) {
    volatile uint16_t* vga = (volatile uint16_t*)VGA_MEM_ADDR;

    // Draw Corners & Horizontal lines
    for (int i = x + 1; i < x + width - 1; i++) {
        if (i >= 0 && i < VGA_WIDTH) {
            if (y >= 0 && y < VGA_HEIGHT) vga[y * VGA_WIDTH + i] = (uint16_t)0xCD | (attribute << 8);         // Double line top
            if ((y + height - 1) >= 0 && (y + height - 1) < VGA_HEIGHT) vga[(y + height - 1) * VGA_WIDTH + i] = (uint16_t)0xCD | (attribute << 8); // Double line bottom
        }
    }

    // Draw Vertical lines
    for (int j = y + 1; j < y + height - 1; j++) {
        if (j >= 0 && j < VGA_HEIGHT) {
            if (x >= 0 && x < VGA_WIDTH) vga[j * VGA_WIDTH + x] = (uint16_t)0xBA | (attribute << 8);         // Vertical left
            if ((x + width - 1) >= 0 && (x + width - 1) < VGA_WIDTH) vga[j * VGA_WIDTH + (x + width - 1)] = (uint16_t)0xBA | (attribute << 8); // Vertical right
        }
    }

    // Setup Box Corner Connectors
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT)                 vga[y * VGA_WIDTH + x] = (uint16_t)0xC9 | (attribute << 8);
    if ((x + width - 1) >= 0 && (x + width - 1) < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) vga[y * VGA_WIDTH + (x + width - 1)] = (uint16_t)0xBB | (attribute << 8);
    if (x >= 0 && x < VGA_WIDTH && (y + height - 1) >= 0 && (y + height - 1) < VGA_HEIGHT) vga[(y + height - 1) * VGA_WIDTH + x] = (uint16_t)0xC8 | (attribute << 8);
    if ((x + width - 1) >= 0 && (x + width - 1) < VGA_WIDTH && (y + height - 1) >= 0 && (y + height - 1) < VGA_HEIGHT) vga[(y + height - 1) * VGA_WIDTH + (x + width - 1)] = (uint16_t)0xBC | (attribute << 8);

    // Print Title if available
    if (title) {
        int title_len = 0;
        while (title[title_len]) title_len++;
        int title_x = x + (width - title_len) / 2;
        tui_print_at(title_x, y, title, attribute);
    }
}
