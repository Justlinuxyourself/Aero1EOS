#include <stdint.h>

#define SCREEN_WIDTH  720
#define SCREEN_HEIGHT 1600

static uint32_t* fb = (uint32_t*)0x50607E00; 

extern unsigned char font_bitmap[128][8];

static int cursor_x = 0;
static int cursor_y = 0;

static inline uint32_t prepare_color_for_mtk(uint32_t rgba) {
    return rgba;
}

void fb_draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        fb[y * SCREEN_WIDTH + x] = prepare_color_for_mtk(color);
    }
}

void fb_clear(uint32_t color) {
    uint32_t target_color = prepare_color_for_mtk(color);
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        fb[i] = target_color;
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

    // Clip screen boundaries safely based on the updated A06 resolution profile
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
