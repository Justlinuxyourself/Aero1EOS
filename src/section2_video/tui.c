#include "tui_video.h"
#include "../section1_cpu/io.h"

static int current_selection = 0;
static int slider_val = 5; // Valid steps: 0 to 9
static uint8_t is_dragging = 0;

static const char* app_labels[6] = {
    "CALCULATOR", "AUDIO OSC ", "FAULT TRAP",
    "FILE INSP ", "PONG GAME ", "TELEMETRY "
};

// Maps slider step (0-9) to a real audio frequency (Hz)
static const uint16_t pitch_frequencies[10] = {
    261, // 0: C4
    294, // 1: D4
    329, // 2: E4
    349, // 3: F4
    392, // 4: G4
    440, // 5: A4 (Default Anchor)
    494, // 6: B4
    523, // 7: C5
    587, // 8: D5
    659  // 9: E5
};

// Updates PIT timer hardware counters directly
static void update_hardware_speaker() {
    if (is_dragging) {
        uint16_t target_hz = pitch_frequencies[slider_val];
        // 1193182 Hz is the internal base oscillator speed of the Intel 8253/8254 PIT chip
        uint16_t divisor = 1193182 / target_hz;

        outb(0x43, 0xB6); // Command register: Channel 2, LSB/MSB, Square Wave Mode
        outb(0x42, (uint8_t)(divisor & 0xFF));        // Send Low Byte
        outb(0x42, (uint8_t)((divisor >> 8) & 0xFF)); // Send High Byte

        // Flip the gate bits on System Control Port B to connect PIT to the speaker
        uint8_t speaker_state = inb(0x61);
        if ((speaker_state & 3) != 3) {
            outb(0x61, speaker_state | 3); // Force Bits 0 and 1 high to un-mute
        }
    } else {
        // Disconnect the oscillator lines to mute the speaker channel cleanly
        outb(0x61, inb(0x61) & 0xFC);
    }
}

static void tui_render_all() {
    tui_clear(ATTR_DEFAULT);

    tui_draw_string(27, 1, "=== ALIX64 4.0 TUI DESKTOP ===", ATTR_BANNER); 
    tui_draw_string(32, 2, "INDEPENDENT CORE OS", ATTR_DEFAULT);

    // App Grid
    for (int i = 0; i < 6; i++) {
        int col = i % 3;
        int row = i / 3;
        int x = 6 + (col * 24);
        int y = 5 + (row * 6);

        uint8_t box_attr = (current_selection == i) ? ATTR_SELECTION : ATTR_BORDER;
        
        tui_draw_window(x, y, 20, 5, (current_selection == i) ? "ACTIVE" : "LAUNCH", box_attr);
        tui_draw_string(x + 5, y + 2, app_labels[i], (current_selection == i) ? ATTR_SELECTION : ATTR_APP_BOX);
    }

    // Parameters Control Block Layout
    tui_draw_window(8, 17, 64, 5, "SYSTEM ATTRIBUTES Panel", ATTR_BORDER);
    tui_draw_string(12, 19, "SPEAKER OSC PITCH:", ATTR_DEFAULT);
    
    uint8_t slider_attr = (current_selection == 6) ? ATTR_SELECTION : ATTR_DEFAULT;
    tui_draw_string(32, 19, "[----------]", slider_attr);
    
    uint8_t knob_attr = (is_dragging) ? ATTR_SELECTION : ATTR_SLIDER; 
    tui_draw_char(33 + slider_val, 19, (char)0xDB, knob_attr); // Render Knob block: █

    if (is_dragging) {
        tui_draw_string(46, 19, "< PITCH LIVE TUNING >", ATTR_BANNER);
    }

    tui_draw_string(2, 23, "NAVIGATE: Arrow Keys | ACTIVATE: Enter/Spacebar", ATTR_FOOTER);
    tui_draw_string(2, 24, "EXIT ENVIRONMENT: Press Escape (ESC)", ATTR_FOOTER);
}

static void launch_app_stub(int slot) {
    tui_clear(ATTR_DEFAULT);
    tui_draw_window(15, 8, 50, 7, "EXECUTION LAUNCHER", ATTR_BORDER);
    tui_draw_string(18, 10, "TARGET ACTIVE:", ATTR_DEFAULT);
    tui_draw_string(33, 10, app_labels[slot], ATTR_BANNER);
    tui_draw_string(18, 12, "Press ANY key to drop back to desktop...", ATTR_APP_BOX);

    for (volatile int d = 0; d < 2000000; d++);

    while (!(inb(0x64) & 1));
    inb(0x60); 

    while (inb(0x64) & 1) { inb(0x60); }
}

void cmd_start_gui() {
    extern void vga_clear();
    extern void vga_write(const char* text);

    volatile int running = 1;

    while (running) {
        tui_render_all();

        while (!(inb(0x64) & 1));
        uint8_t scancode = inb(0x60);
        if (scancode & 0x80) {
            continue; 
        }
        if (scancode == 0x01) { // ESC
            running = 0;
            is_dragging = 0;
            update_hardware_speaker(); // Safeguard: turn sound off
            break;
        }

        if (is_dragging) {
            if (scancode == 0x4B && slider_val > 0) { 
                slider_val--;      
                update_hardware_speaker(); // Update audio frequency live on step drop
            }
            if (scancode == 0x4D && slider_val < 9) { 
                slider_val++;      
                update_hardware_speaker(); // Update audio frequency live on step climb
            }
            if (scancode == 0x39 || scancode == 0x1C) { // Space or Enter
                is_dragging = 0;
                update_hardware_speaker(); // Turns off the oscillator lines when released
            }
        } 
        else {
            if (scancode == 0x4D) { 
                if (current_selection < 6) current_selection++;
            }
            else if (scancode == 0x4B) { 
                if (current_selection > 0) current_selection--;
            }
            else if (scancode == 0x50) { 
                if (current_selection <= 2) current_selection += 3;
                else if (current_selection <= 5) current_selection = 6;
            }
            else if (scancode == 0x48) { 
                if (current_selection == 6) current_selection = 3;
                else if (current_selection >= 3) current_selection -= 3;
            }
            else if (scancode == 0x39 || scancode == 0x1C) { 
                if (current_selection == 6) {
                    is_dragging = 1;
                    update_hardware_speaker(); // Immediately fire tone upon selection hook
                } else {
                    launch_app_stub(current_selection);
                }
            }
        }
        
        for (volatile int d = 0; d < 30000; d++);
    }

    tui_clear(ATTR_DEFAULT);
    vga_clear();
    vga_write("Returned to AliOS Shell, Press Enter.\n");
}
