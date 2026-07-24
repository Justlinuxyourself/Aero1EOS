#include <stdint.h>

#define SCREEN_WIDTH  720
#define SCREEN_HEIGHT 1280

// Base physical pointer mapped via the Qualcomm MDSS display memory layer on MSM8940
static uint32_t* fb = (uint32_t*)0x9D400000; 

extern unsigned char font_bitmap[128][8];

static int cursor_x = 0;
static int cursor_y = 0;

// Converts an incoming standard RGBA hex code safely over to the device's native BGRA channel layout
static inline uint32_t convert_rgba_to_bgra(uint32_t rgba) {
    uint8_t r = (rgba >> 24) & 0xFF;
    uint8_t g = (rgba >> 16) & 0xFF;
    uint8_t b = (rgba >> 8)  & 0xFF;
    uint8_t a = rgba         & 0xFF;
    
    // Package into 0xAARRGGBB format layout expected by legacy Snapdragon panel configurations
    return (a << 24) | (r << 16) | (g << 8) | b;
}

void fb_draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        fb[y * SCREEN_WIDTH + x] = convert_rgba_to_bgra(color);
    }
}

void fb_clear(uint32_t color) {
    uint32_t bgra_color = convert_rgba_to_bgra(color);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        fb[i] = bgra_color;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void fb_putchar(char c, uint32_t fg, uint32_t bg) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y += 10; // Shift down for next line boundary
        return;
    }
    if (c == '\r') {
        cursor_x = 0;
        return;
    }

    // Clip screen boundaries safely
    if (cursor_x + 8 > SCREEN_WIDTH) {
        cursor_x = 0;
        cursor_y += 10;
    }
    if (cursor_y + 8 > SCREEN_HEIGHT) {
        fb_clear(bg); // Reset screen view clear if space runs out
    }

    if (c < 0 || c > 127) return;

    for (int row = 0; row < 8; row++) {
        uint8_t bits = font_bitmap[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                fb_draw_pixel(cursor_x + col, cursor_y + row, fg);
            } else {
                fb_draw_pixel(cursor_x + col, cursor_y + row, bg);
            }
        }
    }
    cursor_x += 8;
}

void fb_puts(const char* str, uint32_t fg, uint32_t bg) {
    while (*str) {
        fb_putchar(*str++, fg, bg);
    }
}
