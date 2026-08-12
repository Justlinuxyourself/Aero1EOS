#include "../section8_global-header/global.h"
/* Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "io.h"


// --- Private State for Background Playback ---
static int current_note_idx = -1;
static unsigned long long next_event_tick = 0;

static Note startup_notes[] = {
    {880, 250}, // High note (A5)
    {0,   30},  // Short gap
    {659, 250}, // Middle note (E5)
    {0,   30},  // Short gap
    {554, 450}, // Lower, sustained note (C#5)
    {-1,  0}    // STOP
};

// --- 1. Core Hardware Control (Same as yours) ---

void nosound() {
    outb(0x61, inb(0x61) & 0xFC);
}

void play_sound(unsigned int nFrequence) {
    if (nFrequence == 0) {
        nosound();
        return;
    }
    unsigned int Div = 1193182 / nFrequence;
    outb(0x43, 0xb6);
    outb(0x42, (unsigned char) (Div & 0xFF));
    outb(0x42, (unsigned char) ((Div >> 8) & 0xFF));

    unsigned char tmp = inb(0x61);
    if (tmp != (tmp | 3)) outb(0x61, tmp | 3);
}



// --- 2. The New Async "Startup" Trigger ---

void startup_melody() {
    current_note_idx = 0;
    next_event_tick = timer_ticks;
}

// --- 3. The New Async "Beep" (Doesn't freeze the shell) ---

void beep() {
        play_sound(1000);
}

void speaker_update() {
    if (current_note_idx == -1) return;

    if (timer_ticks >= next_event_tick) {
        Note n = startup_notes[current_note_idx];

        if (n.freq == -1) {
            nosound();
            current_note_idx = -1;
            return;
        }

        if (n.freq == 0) nosound();
        else play_sound(n.freq);

        // 200Hz logic: ms / 5 = ticks
        next_event_tick = timer_ticks + (n.duration_ms / 5);
        current_note_idx++;
    }
}
