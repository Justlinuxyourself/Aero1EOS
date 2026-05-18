/* src/section2_video/vga.c */
/* LINKED TO NOTEBOOK: SECTION II - Enhanced Video & Independent TTYs */
/* Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "../section1_cpu/heap.h"
#define VGA_ADDRESS 0xB8000
#define NOTEBOOK_YELLOW 0x1E
unsigned char current_vga_color = NOTEBOOK_YELLOW; // Default color
#define WIDTH 80
#define HEIGHT 25
#define MAX_TTYS 10
#define VIDEO_SIZE (WIDTH * HEIGHT * 2)
int status_bar_enabled = 1; // 1 = Enabled, 0 = Disabled/Hidden
extern int cmos_get_hour();
extern int cmos_get_min();
extern int cmos_get_day();
extern int cmos_get_month();
extern int cmos_get_sec();
extern void update_hardware_cursor();
int timezone_offset_seconds = 0; // The "Master" variable

// Independent TTY structure
typedef struct {
    unsigned short* buffer;
    int cursor_pos;
    char command_buffer[80];
    int buffer_idx;
} tty_t;

tty_t ttys[MAX_TTYS];
int current_tty = 0;

// Helper to determine a matching status bar background based on the user's choices
unsigned char get_status_bar_attribute() {
    // Extract the background color chosen by the user (the upper 4 bits)
    unsigned char bg = current_vga_color & 0xF0;
    
    // If their chosen background is black or very dark, use white text on their background.
    // Otherwise, adaptively use black text on their chosen background for high contrast.
    if (bg == 0x00) { 
        return bg | 0x0F; // White text on user's background
    } else {
        return bg | 0x00; // Black text on user's background
    }
}

void vga_write_num_at(int pos, int num) {
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
    unsigned char status_color = get_status_bar_attribute();
    
    vga_hardware[pos] = (unsigned short)((num / 10) + '0') | (unsigned short)status_color << 8;
    vga_hardware[pos + 1] = (unsigned short)((num % 10) + '0') | (unsigned short)status_color << 8;
}

void vga_set_cursor(int x, int y) {
    if (x >= WIDTH) x = WIDTH - 1;
    if (y >= HEIGHT - 1) y = HEIGHT - 2;

    tty_t* active = &ttys[current_tty];
    active->cursor_pos = (y * WIDTH) + x;
    update_hardware_cursor(active->cursor_pos);
}

void vga_set_attribute(unsigned char color) {
    current_vga_color = color; // Sync the global pointer variable
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
    tty_t* active = &ttys[current_tty];

    // OVERWRITE EVERYTHING: Run through all 2000 cells (Rows 0 to 24)
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        // Keep the ASCII character byte, update the color byte
        vga_hardware[i] = (vga_hardware[i] & 0x00FF) | (color << 8);
        active->buffer[i] = (active->buffer[i] & 0x00FF) | (color << 8);
    }

    // Force refresh the status bar immediately so it blends perfectly
    vga_draw_status_bar();
}

void vga_draw_status_bar() {
    if (!status_bar_enabled) {
        return;
    }
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
    tty_t* active = &ttys[current_tty];
    int base_pos = 24 * 80; // Row 24

    // Fetch dynamic bar attribute
    unsigned char status_color = get_status_bar_attribute();

    // Fill the background cells of Row 24 first so the entire bar has a unified layout color
    for (int i = 0; i < WIDTH; i++) {
        vga_hardware[base_pos + i] = (vga_hardware[base_pos + i] & 0x00FF) | (status_color << 8);
        active->buffer[base_pos + i] = (active->buffer[base_pos + i] & 0x00FF) | (status_color << 8);
    }

    int s = cmos_get_sec();
    int m = cmos_get_min();
    int h = cmos_get_hour();

    long total_seconds = (h * 3600) + (m * 60) + s + timezone_offset_seconds;

    while (total_seconds < 0) total_seconds += 86400;
    while (total_seconds >= 86400) total_seconds -= 86400;

    int final_h = total_seconds / 3600;
    int final_m = (total_seconds % 3600) / 60;
    int final_s = total_seconds % 60;

    const char* am_pm = (final_h >= 12) ? "PM" : "AM";
    int hour12 = final_h % 12;
    if (hour12 == 0) hour12 = 12; 

    // Draw Date
    vga_write_num_at(base_pos, cmos_get_month());
    vga_hardware[base_pos + 2] = (unsigned short)'/' | (unsigned short)status_color << 8;
    vga_write_num_at(base_pos + 3, cmos_get_day());

    // Draw Time
    vga_hardware[base_pos + 6] = (unsigned short)'|' | (unsigned short)status_color << 8;
    vga_write_num_at(base_pos + 8, hour12);
    vga_hardware[base_pos + 10] = (unsigned short)':' | (unsigned short)status_color << 8;
    vga_write_num_at(base_pos + 11, final_m);
    vga_hardware[base_pos + 13] = (unsigned short)':' | (unsigned short)status_color << 8;
    vga_write_num_at(base_pos + 14, final_s);
    
    vga_hardware[base_pos + 17] = (unsigned short)am_pm[0] | (unsigned short)status_color << 8;
    vga_hardware[base_pos + 18] = (unsigned short)am_pm[1] | (unsigned short)status_color << 8;

    // Draw TTY ID (Uses contrasting colors tied to status layout)
    int tty_pos = base_pos + 70;
    vga_hardware[tty_pos] = (unsigned short)'T' | (unsigned short)status_color << 8;
    vga_hardware[tty_pos+1] = (unsigned short)'T' | (unsigned short)status_color << 8;
    vga_hardware[tty_pos+2] = (unsigned short)'Y' | (unsigned short)status_color << 8;
    vga_hardware[tty_pos+3] = (unsigned short)':' | (unsigned short)status_color << 8;
    vga_hardware[tty_pos+5] = (unsigned short)(current_tty + '0') | (unsigned short)status_color << 8;

    // Sync hardware state of row 24 back into the current TTY active backup storage
    for (int i = 0; i < WIDTH; i++) {
        active->buffer[base_pos + i] = vga_hardware[base_pos + i];
    }
}

void update_hardware_cursor(int pos) {
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0x0F), "Nd"((unsigned short)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)(pos & 0xFF)), "Nd"((unsigned short)0x3D5));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0x0E), "Nd"((unsigned short)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)((pos >> 8) & 0xFF)), "Nd"((unsigned short)0x3D5));
}

void vga_init_ttys() {
    for(int i = 0; i < MAX_TTYS; i++) {
        ttys[i].buffer = (unsigned short*)kmalloc(VIDEO_SIZE);
        ttys[i].cursor_pos = 0;
        ttys[i].buffer_idx = 0;
        
        for (int j = 0; j < WIDTH * HEIGHT; j++) {
            ttys[i].buffer[j] = (unsigned short)' ' | (unsigned short)NOTEBOOK_YELLOW << 8;
        }
    }
}

void switch_tty(int new_tty) {
    if (new_tty == current_tty) return;
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
    
    for(int i = 0; i < WIDTH * HEIGHT; i++) {
        ttys[current_tty].buffer[i] = vga_hardware[i];
    }
    
    current_tty = new_tty;
    
    for(int i = 0; i < WIDTH * HEIGHT; i++) {
        vga_hardware[i] = ttys[current_tty].buffer[i];
    }
    vga_draw_status_bar();
    update_hardware_cursor(ttys[current_tty].cursor_pos);
}

void vga_clear() {
    unsigned short* vga_ptr = (unsigned short*)VGA_ADDRESS;
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        unsigned short blank = (unsigned short)' ' | (unsigned short)current_vga_color << 8;
        vga_ptr[i] = blank;
        ttys[current_tty].buffer[i] = blank;
    }
    ttys[current_tty].cursor_pos = 0;
    vga_draw_status_bar();
    update_hardware_cursor(0);
}

void vga_scroll() {
    tty_t* active = &ttys[current_tty];
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;

    for (int i = 0; i < (HEIGHT - 2) * WIDTH; i++) {
        active->buffer[i] = active->buffer[i + WIDTH];
    }

    for (int i = (HEIGHT - 2) * WIDTH; i < (HEIGHT - 1) * WIDTH; i++) {
        active->buffer[i] = (unsigned short)' ' | (unsigned short)current_vga_color << 8;
    }

    for (int i = 0; i < WIDTH * (HEIGHT - 1); i++) {
        vga_hardware[i] = active->buffer[i];
    }

    active->cursor_pos = (HEIGHT - 2) * WIDTH;
    vga_draw_status_bar();
}

void vga_set_color(unsigned char color) {
    // Instead of just setting the variable, overwrite everything instantly
    vga_set_attribute(color);
}

void vga_putchar(char c) {
    tty_t* active = &ttys[current_tty];
    unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;

    if (active->cursor_pos >= WIDTH * (HEIGHT - 1)) {
        vga_scroll();
    }

    if (c == '\n') {
        active->cursor_pos += WIDTH - (active->cursor_pos % WIDTH);
    } else if (c == '\b') {
        if (active->cursor_pos > 0) {
            active->cursor_pos--;
            unsigned short blank = (unsigned short)' ' | (unsigned short)current_vga_color << 8;
            vga_hardware[active->cursor_pos] = blank;
            active->buffer[active->cursor_pos] = blank;
        }
    } else {
        unsigned short glyph = (unsigned short)c | (unsigned short)current_vga_color << 8;
        vga_hardware[active->cursor_pos] = glyph;
        active->buffer[active->cursor_pos] = glyph;
        active->cursor_pos++;
    }

    if (active->cursor_pos >= WIDTH * (HEIGHT - 1)) {
        vga_scroll();
    }

    update_hardware_cursor(active->cursor_pos);
}

void vga_write(const char* data) {
    for (int i = 0; data[i] != '\0'; i++) {
        vga_putchar(data[i]);
    }
}
