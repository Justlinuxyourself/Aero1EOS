#include <stdint.h>

#define GUI_MEM ((volatile uint8_t*)0xA0000)
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#include "../section1_cpu_io.h"

// Global Theme Background Colors (VGA DAC constraints: 0 to 63)
static uint8_t bg_r = 12; 
static uint8_t bg_g = 12;
static uint8_t bg_b = 16;

// Slider Coordinate Tracking Positions
static int slider_r_y = 70;
static int slider_g_y = 110;
static int slider_b_y = 50;

// Keyboard UI Navigation State Engine
// Items: 0-5 are Apps, 6 is Red Slider, 7 is Green Slider, 8 is Blue Slider
static int current_selection = 0; 
static uint8_t is_dragging_slider = 0; 

// Hardware Palette Mapping Indices
#define COLOR_BACKGROUND  0x14  
#define COLOR_WHITE       0x0F
#define COLOR_BLACK       0x00
#define COLOR_LIGHT_GRAY  0x07
#define COLOR_SELECTION   0x2A  
#define COLOR_SLIDER_R    0x28
#define COLOR_SLIDER_G    0x2F
#define COLOR_SLIDER_B    0x2D

typedef struct {
    int x, y, w, h;
    const char* label;
} gui_app_t;

static gui_app_t app_grid[6] = {
    {35,  40,  42, 35, "CALC"}, 
    {95,  40,  42, 35, "BEEP"},
    {155, 40,  42, 35, "TRAP"}, 
    {35,  110, 42, 35, "FILE"},
    {95,  110, 42, 35, "GAME"},
    {155, 110, 42, 35, "STAT"}
};

// Complete 5x7 Bitmap Font Table for the whole Alphabet (A-Z) and numbers (0-9)
static const uint8_t font_alpha[38][7] = {
    {0x04, 0x0A, 0x11, 0x1F, 0x11, 0x11, 0x11}, // A
    {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // B
    {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // C
    {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, // D
    {0x1F, 0x10, 0x10, 0x1F, 0x10, 0x10, 0x1F}, // E
    {0x1F, 0x10, 0x10, 0x1F, 0x10, 0x10, 0x10}, // F
    {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // G
    {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // H
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // I
    {0x1F, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, // J
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // L
    {0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11}, // M
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, // N
    {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // O
    {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // P
    {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // Q
    {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // R
    {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}, // S
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // T
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // U
    {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04}, // V
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}, // W
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // X
    {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // Y
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // Z
    {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
    {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
    {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}, // 2
    {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // 3
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
    {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 9
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // [Space]
};

// Graphics Vector Assembly Primitives
void draw_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        GUI_MEM[y * SCREEN_WIDTH + x] = color;
    }
}

void draw_rect(int start_x, int start_y, int width, int height, uint8_t color) {
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) {
            draw_pixel(x, y, color);
        }
    }
}

void draw_triangle_down(int tip_x, int tip_y, int size, uint8_t color) {
    for (int i = 0; i < size; i++) {
        for (int j = tip_x - (size - i); j <= tip_x + (size - i); j++) {
            draw_pixel(j, tip_y - i, color);
        }
    }
}

void draw_vline(int x, int start_y, int end_y, uint8_t color) {
    for (int y = start_y; y <= end_y; y++) {
        draw_pixel(x, y, color);
    }
}

void draw_string(int x, int y, const char* str, uint8_t color) {
    for (int i = 0; str[i] != '\0'; i++) {
        char c = str[i];
        int letter_idx = 37; // Default to space character
        
        if (c >= 'a' && c <= 'z') c -= 32;
        
        if (c >= 'A' && c <= 'Z') {
            letter_idx = c - 'A';
        } else if (c >= '0' && c <= '9') {
            letter_idx = 26 + (c - '0');
        }

        int char_x = x + (i * 7);
        for (int row = 0; row < 7; row++) {
            uint8_t row_bits = font_alpha[letter_idx][row];
            for (int col = 0; col < 5; col++) {
                if (row_bits & (0x10 >> col)) {
                    draw_pixel(char_x + col, y + row, color);
                }
            }
        }
    }
}

void update_vga_background_color() {
    outb(0x3C8, COLOR_BACKGROUND); 
    outb(0x3C9, bg_r);             
    outb(0x3C9, bg_g);             
    outb(0x3C9, bg_b);             
}


void execute_app(int slot) {
    // APP 1: CALCULATOR
    if (slot == 0) {
        int count = 5;
        char count_str[2] = "5";
        draw_rect(15, 15, 290, 170, COLOR_BLACK);
        draw_string(25, 25, "CALC ACTIVE - UP AND DOWN ARROWS", COLOR_WHITE);
        draw_string(25, 45, "PRESS ESC TO EXIT", COLOR_LIGHT_GRAY);
        
        while (1) {
            draw_rect(100, 80, 50, 30, COLOR_BLACK);
            count_str[0] = '0' + (count % 10);
            draw_string(120, 90, count_str, COLOR_SELECTION);
            
            while (!(inb(0x64) & 1));
            uint8_t key = inb(0x60);
            if (key == 0x01) break; // ESC
            if (key == 0x48 && count < 9) count++; // Up
            if (key == 0x50 && count > 0) count--; // Down
            for(volatile int d=0; d<10000; d++);
        }
    } 
    // APP 2: AUDIO OSCILLATOR BEEP
    else if (slot == 1) {
        uint16_t freq_divisor = 1193; // Generates roughly 1kHz tone
        draw_rect(15, 15, 290, 170, COLOR_BLACK);
        draw_string(25, 25, "AUDIO OSCILLATOR SPEAKER ACTIVE", COLOR_WHITE);
        draw_string(25, 40, "UP AND DOWN TUNES PITCh - SPACE OFF", COLOR_LIGHT_GRAY);
        
        // Setup PIT Channel 2 Speaker Hardware Timer 
        outb(0x43, 0xB6);
        
        while (1) {
            outb(0x42, (uint8_t)(freq_divisor & 0xFF));
            outb(0x42, (uint8_t)((freq_divisor >> 8) & 0xFF));
            outb(0x61, inb(0x61) | 3); // Sound on
            
            while (!(inb(0x64) & 1));
            uint8_t key = inb(0x60);
            if (key == 0x01) break; // ESC
            if (key == 0x48 && freq_divisor > 200) freq_divisor -= 100; // Pitch up
            if (key == 0x50 && freq_divisor < 5000) freq_divisor += 100; // Pitch down
            if (key == 0x39) outb(0x61, inb(0x61) & 0xFC); // Mute sound output
        }
        outb(0x61, inb(0x61) & 0xFC); // Safeguard Speaker Off
    } 
    // APP 3: CORE FAULT EXCEPTION TRAP
    else if (slot == 2) {
        draw_rect(15, 15, 290, 170, COLOR_BLACK);
        draw_string(25, 40, "TRAPPING CPU HANDLER VECTOR 0X00", COLOR_SLIDER_R);
        for(volatile int d=0; d<8000000; d++);
        
        volatile int zero_value = 0;
        volatile int crash_trigger = 100 / zero_value;
        (void)crash_trigger;
    }
    // APP 4: HEX STORAGE SECTOR FILE INSPECTOR
    else if (slot == 3) {
        draw_rect(15, 15, 290, 170, COLOR_BLACK);
        draw_string(25, 25, "RAW KERNEL SECTOR MEMORY STREAM", COLOR_WHITE);
        
        // Print raw hex layout configurations straight off BIOS video buffers
        for (int row = 0; row < 6; row++) {
            volatile uint8_t* ptr = (volatile uint8_t*)(0x10000 + (row * 4));
            char hex_display[9] = "00000000";
            for (int b = 0; b < 4; b++) {
                uint8_t val = ptr[b];
                hex_display[b*2]   = '0' + (val >> 4);
                hex_display[b*2+1] = '0' + (val & 0x0F);
            }
            draw_string(35, 50 + (row * 15), hex_display, COLOR_SLIDER_G);
        }
        while (1) {
            if (inb(0x64) & 1) { if (inb(0x60) == 0x01) break; }
        }
    }
    // APP 5: RETRO PONG ACTION GAME
    else if (slot == 4) {
        int paddle_y = 70;
        int ball_x = 100, ball_y = 80;
        int ball_dx = 3, ball_dy = 2;
        
        while (1) {
            draw_rect(15, 15, 290, 170, COLOR_BLACK);
            draw_string(25, 22, "PONG GAME - UP/DOWN ARROWS", COLOR_WHITE);
            
            // Render Entities
            draw_rect(20, paddle_y, 6, 30, COLOR_SELECTION);
            draw_rect(ball_x, ball_y, 4, 4, COLOR_WHITE);
            
            // Physics Simulation Mechanics
            ball_x += ball_dx; ball_y += ball_dy;
            if (ball_y <= 40 || ball_y >= 170) ball_dy = -ball_dy;
            if (ball_x >= 280) ball_dx = -ball_dx;
            
            // Paddle Collisions Check Bounds
            if (ball_x <= 26 && ball_y >= paddle_y && ball_y <= (paddle_y + 30)) {
                ball_dx = -ball_dx;
            } else if (ball_x < 15) {
                // Ball resets if missed
                ball_x = 150; ball_y = 80;
            }
            
            if (inb(0x64) & 1) {
                uint8_t key = inb(0x60);
                if (key == 0x01) break; // Exit game
                if (key == 0x48 && paddle_y > 42) paddle_y -= 6;
                if (key == 0x50 && paddle_y < 140) paddle_y += 6;
            }
            for (volatile int delay = 0; delay < 120000; delay++);
        }
    }
    // APP 6: HARDWARE TELEMETRY STAT ENGINE
    else if (slot == 5) {
        draw_rect(15, 15, 290, 170, COLOR_BLACK);
        draw_string(25, 25, "ALIOS TELEMETRY SNAPSHOT MONITOR", COLOR_WHITE);
        
        draw_string(35, 55,  "OS MODEL STATUS   ACTIVE CORE", COLOR_SLIDER_B);
        draw_string(35, 75,  "DISPLAY CONTROLLER PRO VGA MODE 13H", COLOR_SLIDER_B);
        draw_string(35, 95,  "EXECUTION STACK   PROTECTED X86", COLOR_SLIDER_B);
        draw_string(35, 115, "ACTIVE SYSTEM ID  4006", COLOR_SLIDER_B);
        
        while (1) {
            if (inb(0x64) & 1) { if (inb(0x60) == 0x01) break; }
        }
    }
}

void render_interface() {
    draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, COLOR_BACKGROUND);

    draw_string(110, 12, "ALIOS SYSTEM", COLOR_WHITE);
    draw_string(125, 22, "SOLO DEV", COLOR_LIGHT_GRAY);

    // Render out App Launcher Grid Layout 
    for (int i = 0; i < 6; i++) {
        uint8_t border_color = (current_selection == i) ? COLOR_SELECTION : COLOR_LIGHT_GRAY;

        draw_rect(app_grid[i].x, app_grid[i].y, app_grid[i].w, app_grid[i].h, border_color);
        draw_rect(app_grid[i].x + 2, app_grid[i].y + 2, app_grid[i].w - 4, app_grid[i].h - 4, COLOR_WHITE);
        
        draw_string(app_grid[i].x + 7, app_grid[i].y + 14, app_grid[i].label, COLOR_BLACK);
        draw_triangle_down(app_grid[i].x + (app_grid[i].w / 2), app_grid[i].y + app_grid[i].h + 8, 6, border_color);
    }

    // Stylus Separator Handle Element
    draw_rect(215, 40, 6, 110, COLOR_WHITE);
    draw_pixel(217, 39, COLOR_BLACK); draw_pixel(217, 151, COLOR_BLACK);

    // RGB Slider Track Control Engine Interfaces
    uint8_t r_knob_color = (current_selection == 6) ? COLOR_SELECTION : COLOR_WHITE;
    uint8_t g_knob_color = (current_selection == 7) ? COLOR_SELECTION : COLOR_WHITE;
    uint8_t b_knob_color = (current_selection == 8) ? COLOR_SELECTION : COLOR_WHITE;

    if (is_dragging_slider) {
        draw_string(235, 175, "DRAG ACTIVE", COLOR_WHITE);
    }

    draw_vline(245, 30, 160, COLOR_SLIDER_R);
    draw_rect(241, slider_r_y, 9, 9, r_knob_color); 

    draw_vline(265, 30, 160, COLOR_SLIDER_G);
    draw_rect(261, slider_g_y, 9, 9, g_knob_color); 

    draw_vline(285, 30, 160, COLOR_SLIDER_B);
    draw_rect(281, slider_b_y, 9, 9, b_knob_color); 
}

void cmd_start_gui() {
    extern void set_gui_mode();
    extern void set_text_mode();
    extern void vga_clear();

    set_gui_mode();
    update_vga_background_color();

    volatile int running = 1;
    while (running) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);

            if (scancode == 0x01) {
                running = 0;
            }

            if (is_dragging_slider) {
                if (scancode == 0x48) { // Up Arrow
                    if (current_selection == 6 && slider_r_y > 30) { slider_r_y -= 4; bg_r = ((151 - slider_r_y) * 63) / 121; }
                    if (current_selection == 7 && slider_g_y > 30) { slider_g_y -= 4; bg_g = ((151 - slider_g_y) * 63) / 121; }
                    if (current_selection == 8 && slider_b_y > 30) { slider_b_y -= 4; bg_b = ((151 - slider_b_y) * 63) / 121; }
                    update_vga_background_color();
                }
                else if (scancode == 0x50) { // Down Arrow
                    if (current_selection == 6 && slider_r_y < 151) { slider_r_y += 4; bg_r = ((151 - slider_r_y) * 63) / 121; }
                    if (current_selection == 7 && slider_g_y < 151) { slider_g_y += 4; bg_g = ((151 - slider_g_y) * 63) / 121; }
                    if (current_selection == 8 && slider_b_y < 151) { slider_b_y += 4; bg_b = ((151 - slider_b_y) * 63) / 121; }
                    update_vga_background_color();
                }
                else if (scancode == 0x39 || scancode == 0x1C) { 
                    is_dragging_slider = 0;
                }
            } 
            else {
                if (scancode == 0x4D) { // Right Arrow
                    if (current_selection < 8) current_selection++;
                }
                else if (scancode == 0x4B) { // Left Arrow
                    if (current_selection > 0) current_selection--;
                }
                else if (scancode == 0x50) { // Down Arrow
                    if (current_selection >= 0 && current_selection <= 2) current_selection += 3;
                }
                else if (scancode == 0x48) { // Up Arrow
                    if (current_selection >= 3 && current_selection <= 5) current_selection -= 3;
                }
                else if (scancode == 0x39 || scancode == 0x1C) { 
                    if (current_selection >= 0 && current_selection <= 5) {
                        execute_app(current_selection);
                    } else {
                        is_dragging_slider = 1; 
                    }
                }
            }
        }

        render_interface();
        for (volatile int d = 0; d < 45000; d++);
    }

    set_text_mode();
    vga_clear();
    vga_write("Returned to AliOS 4.0 Secure Terminal Shell Session.\n");
    vga_write("> ");
}
