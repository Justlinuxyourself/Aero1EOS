#ifndef TUI_VIDEO_H
#define TUI_VIDEO_H

#include <stdint.h>

#define COLS 80
#define ROWS 24

// Base Color Scheme Definitions (VGA Standard text mode attributes)
#define ATTR_DEFAULT      0x1E // Blue on Yellow
#define ATTR_SELECTION    0x70 // Inverted: Black on Light Gray
#define ATTR_BORDER       0x09 // Light Blue on Black
#define ATTR_SLIDER       0x0C // Light Red on Black
#define ATTR_APP_BOX      0x0F // Bright White on Black
#define ATTR_BANNER       0x0E // Yellow on Black
#define ATTR_FOOTER       0x0A // Light Green on Black

// Function Prototypes
void tui_clear(uint8_t attribute);
void tui_draw_char(int x, int y, char c, uint8_t attribute);
void tui_draw_string(int x, int y, const char* str, uint8_t attribute);
void tui_draw_window(int start_x, int start_y, int w, int h, const char* title, uint8_t attr);

#endif
