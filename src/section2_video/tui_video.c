#include "tui_video.h"

#define TEXT_MEM ((volatile uint16_t*)0xB8000)

void tui_clear(uint8_t attribute) {
    uint16_t clear_cell = (uint16_t)' ' | ((uint16_t)attribute << 8);
    for (int i = 0; i < COLS * ROWS; i++) {
        TEXT_MEM[i] = clear_cell;
    }
}

void tui_draw_char(int x, int y, char c, uint8_t attribute) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        uintptr_t offset = ((uintptr_t)y * (uintptr_t)COLS) + (uintptr_t)x;
        TEXT_MEM[offset] = (uint16_t)c | ((uint16_t)attribute << 8);
    }
}

void tui_draw_string(int x, int y, const char* str, uint8_t attribute) {
    for (int i = 0; str[i] != '\0'; i++) {
        tui_draw_char(x + i, y, str[i], attribute);
    }
}

void tui_draw_window(int start_x, int start_y, int w, int h, const char* title, uint8_t attr) {
    for (int x = start_x; x < start_x + w; x++) {
        tui_draw_char(x, start_y, (char)0xCD, attr);         
        tui_draw_char(x, start_y + h - 1, (char)0xCD, attr); 
    }
    for (int y = start_y; y < start_y + h; y++) {
        tui_draw_char(start_x, y, (char)0xBA, attr);         
        tui_draw_char(start_x + w - 1, y, (char)0xBA, attr); 
    }
    tui_draw_char(start_x, start_y, (char)0xC9, attr);                 
    tui_draw_char(start_x + w - 1, start_y, (char)0xBB, attr);         
    tui_draw_char(start_x, start_y + h - 1, (char)0xC8, attr);         
    tui_draw_char(start_x + w - 1, start_y + h - 1, (char)0xBC, attr); 

    if (title != 0) {
        int title_len = 0;
        while (title[title_len] != '\0') title_len++;

        tui_draw_char(start_x + 2, start_y, '[', attr);
        tui_draw_string(start_x + 3, start_y, title, ATTR_APP_BOX);
        tui_draw_char(start_x + 3 + title_len, start_y, ']', attr);
    }
}
