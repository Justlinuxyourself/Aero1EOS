  /* 
Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "shell.h"
#include "../section1_cpu/heap.h"
#include "../section1_cpu/io.h"
#include "aliscr.h"
#include "frames.h"
#include "../section3_io/alifs.h"
#include <stdint.h>
#define NOTEBOOK_YELLOW 0x1E
#define VGA_ADDRESS 0xB8000
volatile int is_sleeping = 0;
static command_node_t* command_list = 0;
extern unsigned int get_uptime_seconds();
extern unsigned int get_total_ram_bytes();
extern void lock_system_hardened();
extern int timezone_offset_seconds;
extern void vga_draw_status_bar();
extern void play_sound();
extern void nosound();
extern void vga_set_cursor();
extern void sleep();
extern void cmd_install_os();
extern void ide_read_sector_bytes(uint32_t lba, uint8_t* buffer);
extern void ide_write_sector_bytes(uint32_t lba, uint8_t* buffer);
extern void vga_set_color(unsigned char color);
extern int cmos_get_sec();
extern void sleep_ms();
extern uint32_t ide_get_total_sectors();
extern unsigned int get_uptime_ms();
extern char wait_for_key();
extern void cmd_start_gui();
extern int strncmp(const char* s1, const char* s2, int n);
int alifs_is_directory(char* name);
// Simple PRNG state
static uint32_t next_rand = 1;
// kernel start and end
extern uint8_t _kernel_start;
extern uint8_t _kernel_end;
char current_path[256] = "/"; // Start at root
typedef struct {
    char key[16];
    char value[32];
    int active;
} env_var_t;

env_var_t env_table[10]; // Store up to 10 variables in RAM
// some structs are down with the code that uses it bc i didnt plan for it, it just popped up in my head
typedef struct {
    char task[48];
    int done;
    int active;
} todo_t;

todo_t my_list[10]; // 10 slots for your daily goals

typedef struct {
    const char* ar;
    const char* en;
    const char* meaning;
} name_99_t;

typedef struct {
    const char* book;
    int chapter;
    int verse;
    const char* text;
} bible_t;

typedef struct {
    int surah;
    int ayah;
    const char* text;
} ayah_t;

/* --- String Helpers --- */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n--) {
        if (*s1 != *s2++) return *(unsigned char*)s1 - *(unsigned char*)--s2;
        if (*s1++ == 0) break;
    }
    return 0;
}

int strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}
/* Helper: Reverse a string in place */
void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

/* itoa: Convert integer to string (Base 10 only) */
char* itoa(int value, char* str) {
    int i = 0;
    int isNegative = 0;

    /* Handle 0 explicitly */
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    /* Handle negative numbers */
    if (value < 0) {
        isNegative = 1;
        value = -value;
    }

    /* Process individual digits in Base 10 */
    while (value != 0) {
        int rem = value % 10;
        str[i++] = rem + '0';
        value = value / 10;
    }

    if (isNegative) {
        str[i++] = '-';
    }

    str[i] = '\0';
    reverse(str, i);

    return str;
}
// Simple atoi implementation to convert string to integer
int atoi_custom(char* str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i++;
    } else if (str[0] == '+') {
        i++; // Just skip the plus sign and stay positive
    }

    for (; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9') break;
        res = res * 10 + str[i] - '0';
    }
    return sign * res;
}
/* itohex: Convert integer to Hexadecimal string */
char* itohex(unsigned long value, char* str) {
    char* hex_chars = "0123456789ABCDEF";
    int i = 0;
    
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    while (value > 0) {
        str[i++] = hex_chars[value % 16];
        value /= 16;
    }
    str[i] = '\0';
    reverse(str, i);
    return str;
}

/* --- Built-in Commands --- */
void cmd_help(char* args) {
    (void)args;
    
    vga_write("\nAliOS 4 Commands: ");
    
    command_node_t* curr = command_list;
    while (curr) {
        vga_write(curr->name);
        vga_write(" - ");
        vga_write(curr->description);
        
        curr = curr->next;
        
        if (curr) {
            vga_write(" ||| ");
        }
    }
    vga_write("\n"); // Final newline to keep the shell prompt clean
}


void cmd_cls(char* args) {
    vga_clear();

}

void cmd_echo(char* args) {
    if (args) {
//        vga_write("\n");
        vga_write(args);
    }
}

void cmd_neofetch(char* args) {
    char cpu_name[49];
    get_cpu_name(cpu_name); 
    char mem_str[16];
    itoa(get_heap_usage(), mem_str);
    
    // Get the actual hex address of the heap
    char heap_addr_buf[16];
    itohex((unsigned long)&_kernel_end, heap_addr_buf);
    
    vga_write("   ______      AliOS 4.0\n");
    vga_write("  / ____/      ----------\n");
    vga_write(" / /  __       CPU: "); vga_write(cpu_name); vga_write("\n");
    vga_write("/ /__/ /       MEM: "); vga_write(mem_str); vga_write(" bytes used\n");
    
    // NO MORE HARDCODING: Show the real address
    vga_write("\\____ /        HEAP: 0x"); vga_write(heap_addr_buf); vga_write("\n");
    vga_write("               MODE: 64-bit Long Mode\n");
}

void cmd_uptime(char* args) {
    char sec_str[16];
    itoa(get_uptime_seconds(), sec_str); // Use the real timer data
    
    vga_write("uptime: ");
    vga_write(sec_str);
    vga_write(" seconds.\n");
}
void cmd_free(char* args) {
    unsigned int total = (unsigned int)get_total_ram_bytes();
    unsigned int used = get_heap_usage();
    unsigned int free = total - used;

    char t_str[16], u_str[16], f_str[16];
    
    // No more dividing by 1024!
    itoa(total, t_str);
    itoa(used, u_str);
    itoa(free, f_str);

    vga_write("\nMemory Usage (Bytes):");
    vga_write("\n  Total: "); vga_write(t_str);
    vga_write("\n  Used:  "); vga_write(u_str);
    vga_write("\n  Free:  "); vga_write(f_str);
    vga_write("\n");
}
 
void shell_cmd_timezone(char* arg) {
    if (arg == 0 || arg[0] == '\0') {
        vga_write("Usage: timezone [hours] [seconds]\n");
        return;
    }

    int h = 0, s = 0;
    char* second_part = 0;

    // Split "hours" and "seconds"
    for (int i = 0; arg[i]; i++) {
        if (arg[i] == ' ') {
            arg[i] = '\0';
            second_part = &arg[i+1];
            break;
        }
    }

    h = atoi_custom(arg); 
    if (second_part) s = atoi_custom(second_part);

    // Logic: If hours are negative, seconds should usually be subtracted too
    // Example: GMT-5:30 means -5 hours AND -30 minutes
    int total;
    if (h < 0) {
        total = (h * 3600) - s; 
    } else {
        total = (h * 3600) + s;
    }

    timezone_offset_seconds = total;

    vga_write("Timezone offset set to ");
    char buf[16];
    vga_write(itoa(timezone_offset_seconds, buf));
    vga_write(" seconds.\n");


    // CRITICAL: Refresh the screen so you see the change!
    vga_draw_status_bar(); 
}

void shell_lock() {
    lock_system_hardened();
}
void cmd_test(char* args) {
    vga_write("Calibrating timer (5s wait)...\n");
    for(int i = 5; i > 0; i--) {
        char buf[4];
        itoa(i, buf);
        vga_write(buf);
        vga_write("... ");
        sleep(1);
    }
    vga_write("\nTest complete.\n");
}
void cmd_beep(){
    vga_write("Beeping...");
    play_sound(1000);
    sleep(1);
    nosound();
}
void cmd_about_dev() {
	vga_write("Hi my name is ali, i am 12 years old, and i like planes :3");
}
void draw_custom_plane() {
    int col = 25; // Center it
    
    vga_set_cursor(col, 5);  vga_write("            __\\/__");
    vga_set_cursor(col, 6);  vga_write("           `==/\\==` ");
    vga_set_cursor(col, 7);  vga_write(" ____________/__\\____________");
    vga_set_cursor(col, 8);  vga_write("/____________________________\\");
    vga_set_cursor(col, 9);  vga_write("  __||__||__/.--.\\__||__||__");
    vga_set_cursor(col, 10); vga_write(" /__|___|___( >< )___|___|__\\");
    vga_set_cursor(col, 11); vga_write("           _/`--`\\_");
    vga_set_cursor(col, 12); vga_write("          (/------\\)");
}

void twins() {
    vga_write("\n--- THE TWINS & LEGENDS ---\n");
    
    const char* names[] = {
        "APHRODITE", "SAKI (BEST SUNDAY GOONER)", "QOQO", "KEI", "LWAH", 
        "O1", "O2", "RAYA THE KARAOKE QUEEN", "SEL", "ISHI", "ZAZA", 
        "VANILLA & MAX", "SUSTUBE", "ADNAN", "ZIKE (FAF)", "MOLY", 
        "ROSIE", "E", "Sillycat", "Kaisi", "ZANNNNNNNN", "ABDUALLAH", 
        "MIKAY (BATTERY EATER TWINIES)", "ALIYAH", "AYAH", "DANIEL", 
        "ASEEL (MY SIS)", "KHAILD", "AMAL (ASEELS BSF)", "FATMAH/FARAH (MY AUNT)", 
        "HEAIM", "BASMA & MALAK", "OSAMA & SHERIF (PEAKEST UNCLES 4EVER)", 
        "MARCEL", "FERIBSD", "SUMY", "KYOO", "LEXUS", "RAYYAN", "SPECIAL: GaroDaemon"
    };

    int total_names = sizeof(names) / sizeof(names[0]);

    for (int i = 0; i < total_names; i++) {
        vga_write(names[i]);
        if (i < total_names - 1) {
            vga_write(" ||| ");
        }
    }
    vga_write("\n---------------------------\n");
}


void sys_sleep() {
    is_sleeping = 1;
    vga_clear();
    vga_write("SYSTEM SLEEPING... Press ANY key to wake.");

    // We CANNOT use hlt if we don't have ISRs set up.
    // Instead, we just loop and poll the keyboard.
    while(is_sleeping) {
        // Look at the keyboard status port (0x64)
        // Bit 0 is set if there is data in the buffer
        if (inb(0x64) & 1) {
            unsigned char scan = inb(0x60);
            if (scan < 0x80) { // Any key "Make" code
                is_sleeping = 0;
            }
        }
        
        // Give the CPU a tiny rest without fully halting
        __asm__ volatile("pause"); 
    }

    vga_clear();
    vga_draw_status_bar();
    lock_system_hardened();
}
void command_calc(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: calc [num1] [op] [num2]\n");
        vga_write("Example: calc 5 a 10\n");
        return;
    }

    char* part1 = args;
    char* part2 = 0;
    char* part3 = 0;

    // 1. Find the first space to get the operator
    for (int i = 0; args[i]; i++) {
        if (args[i] == ' ') {
            args[i] = '\0';     // Terminate num1
            part2 = &args[i+1]; // Start of operator
            break;
        }
    }

    if (!part2) { vga_write("Error: Missing operator.\n"); return; }

    // 2. Find the second space to get the second number
    for (int i = 0; part2[i]; i++) {
        if (part2[i] == ' ') {
            part2[i] = '\0';    // Terminate operator
            part3 = &part2[i+1]; // Start of num2
            break;
        }
    }

    if (!part3) { vga_write("Error: Missing second number.\n"); return; }

    int n1 = atoi_custom(part1);
    char op = part2[0]; 
    int n2 = atoi_custom(part3);
    int result = 0;

    if (op == 'a')      result = n1 + n2;
    else if (op == 's') result = n1 - n2;
    else if (op == 'm') result = n1 * n2;
    else if (op == 'd') {
        if (n2 == 0) { vga_write("Error: Div by 0\n"); return; }
        result = n1 / n2;
    } else {
        vga_write("Error: Use a/s/m/d\n");
        return;
    }

    char res_buffer[32];
    itoa(result, res_buffer);
    
    vga_write("Result: ");
    vga_write(res_buffer);
    vga_write("\n");
}

void cmd_peek(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: peek [hex_address]\nExample: peek 0xB8000\n");
        return;
    }

    // Simple hex string to long converter
    unsigned long addr = 0;
    int start = 0;
    if (args[0] == '0' && (args[1] == 'x' || args[1] == 'X')) start = 2;

    for (int i = start; args[i] != '\0'; i++) {
        addr *= 16;
        if (args[i] >= '0' && args[i] <= '9') addr += (args[i] - '0');
        else if (args[i] >= 'a' && args[i] <= 'f') addr += (args[i] - 'a' + 10);
        else if (args[i] >= 'A' && args[i] <= 'F') addr += (args[i] - 'A' + 10);
    }

    unsigned char* ptr = (unsigned char*)addr;
    vga_write("Memory at 0x");
    vga_write(args);
    vga_write(": ");

    // Show the next 8 bytes
    for (int i = 0; i < 8; i++) {
        char buf[3];
        itohex(ptr[i], buf);
        if (ptr[i] < 16) vga_write("0"); // Padding
        vga_write(buf);
        vga_write(" ");
    }
}
void cmd_poke(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: poke [addr] [val]\nExample: poke 0xB8000 0x41\n");
        return;
    }

    char* addr_str = args;
    char* val_str = 0;

    // 1. Split the string at the space
    for (int i = 0; args[i]; i++) {
        if (args[i] == ' ') {
            args[i] = '\0';
            val_str = &args[i+1];
            break;
        }
    }

    if (!val_str) {
        vga_write("Error: Missing value.\n");
        return;
    }

    // 2. Parse Address (Hex or Dec)
    unsigned long addr = 0;
    int i = 0;
    if (addr_str[0] == '0' && (addr_str[1] == 'x' || addr_str[1] == 'X')) {
        i = 2;
        while (addr_str[i]) {
            addr *= 16;
            if (addr_str[i] >= '0' && addr_str[i] <= '9') addr += (addr_str[i] - '0');
            else if (addr_str[i] >= 'a' && addr_str[i] <= 'f') addr += (addr_str[i] - 'a' + 10);
            else if (addr_str[i] >= 'A' && addr_str[i] <= 'F') addr += (addr_str[i] - 'A' + 10);
            i++;
        }
    } else {
        addr = (unsigned long)atoi_custom(addr_str);
    }

    // 3. Parse Value (Hex or Dec)
    unsigned char val = 0;
    if (val_str[0] == '0' && (val_str[1] == 'x' || val_str[1] == 'X')) {
        int j = 2;
        while (val_str[j]) {
            val *= 16;
            if (val_str[j] >= '0' && val_str[j] <= '9') val += (val_str[j] - '0');
            else if (val_str[j] >= 'a' && val_str[j] <= 'f') val += (val_str[j] - 'a' + 10);
            else if (val_str[j] >= 'A' && val_str[j] <= 'F') val += (val_str[j] - 'A' + 10);
            j++;
        }
    } else {
        val = (unsigned char)atoi_custom(val_str);
    }

    // 4. The Poke: Write to raw memory
    unsigned char* ptr = (unsigned char*)addr;
    *ptr = val;

    vga_write("Memory modified at 0x");
    vga_write(addr_str);
    vga_write("\n");
}

void cmd_ayah() {
    // Array of Ayahs stored in the Kernel Data Segment
    ayah_t quran_db[] = {
        {94, 5, "For indeed, with hardship [will be] ease."},
        {2, 152, "So remember Me; I will remember you."},
        {3, 139, "So do not weaken and do not grieve."},
        {2, 286, "Allah does not charge a soul except with that within its capacity."},
        {50, 16, "And We are closer to him than [his] jugular vein."}
    };

    // Calculate total entries in the database
    int db_size = sizeof(quran_db) / sizeof(ayah_t);

    // Use CMOS seconds to pick a random index
    int r = cmos_get_sec() % db_size;

    // Buffers for itoa conversion
    char s_str[8], a_str[8];

    vga_write("\n");
    // Print Format -> (SurahNum):(AyahNum) (Text)
    vga_write(itoa(quran_db[r].surah, s_str));
    vga_write(":");
    vga_write(itoa(quran_db[r].ayah, a_str));
    vga_write(" ");
    vga_write(quran_db[r].text);
    vga_write("\n");
}

void cmd_verse() {
    bible_t bible_db[] = {
        {"Psalms", 23, 1, "The Lord is my shepherd; I shall not want."},
        {"John", 1, 5, "The light shines in the darkness, and the darkness has not overcome it."},
        {"Philippians", 4, 13, "I can do all things through Christ who strengthens me."},
        {"Matthew", 5, 9, "Blessed are the peacemakers, for they shall be called sons of God."},
        {"Proverbs", 3, 5, "Trust in the Lord with all your heart and lean not on your own understanding."}
    };

    int db_size = sizeof(bible_db) / sizeof(bible_t);
    int r = cmos_get_sec() % db_size;

    char c_str[8], v_str[8];

    vga_write("\n");
    // Format: Book Chapter:Verse - Text
    vga_write(bible_db[r].book);
    vga_write(" ");
    vga_write(itoa(bible_db[r].chapter, c_str));
    vga_write(":");
    vga_write(itoa(bible_db[r].verse, v_str));
    vga_write(" - ");
    vga_write(bible_db[r].text);
    vga_write("\n");
}
void cmd_set(char* args) {
    if (args == 0 || *args == '\0') {
        vga_write("Usage: set [key] [value]\n");
        return;
    }

    char* key = args;
    char* val = 0;
    
    for (int i = 0; args[i]; i++) {
        if (args[i] == ' ') {
            args[i] = '\0'; 
            val = &args[i+1]; 
            break;
        }
    }

    if (!val || *val == '\0') {
        vga_write("Error: Missing value for variable.\n");
        return;
    }

    // Save to the table
    for(int i = 0; i < 10; i++) {
        // Look for empty slot or overwrite existing key
        if(!env_table[i].active || strcmp(env_table[i].key, key) == 0) {
            strcpy(env_table[i].key, key);
            strcpy(env_table[i].value, val);
            env_table[i].active = 1;
            vga_write("Variable set.");
            return;
        }
    }
    vga_write("Error: Environment table full!");
}

void cmd_get(char* key) {
    // 1. SAFETY CHECK: If user just types 'get' with no name
    if (key == 0 || key[0] == '\0') {
        vga_write("Usage: get [key]\n");
        return;
    }

    for(int i = 0; i < 10; i++) {
        if(env_table[i].active) {
            // 2. Double check the stored key exists before comparing
            if (env_table[i].key[0] != '\0' && strcmp(env_table[i].key, key) == 0) {
                vga_write(env_table[i].value);
                vga_write("\n");
                return;
            }
        }
    }
    vga_write("Error: Variable not found.\n");
}

void todo_add(char* text) {
    for(int i = 0; i < 10; i++) {
        if(!my_list[i].active) {
            strcpy(my_list[i].task, text);
            my_list[i].done = 0;
            my_list[i].active = 1;
            vga_write("Task added to AliOS list.\n");
            return;
        }
    }
    vga_write("Error: Your brain (list) is full!\n");
}
void todo_show() {
    int found = 0;
    for(int i = 0; i < 10; i++) {
        // Only print if the slot is explicitly marked active
        if(my_list[i].active == 1 && my_list[i].task[0] != '\0') {
            vga_write("- ");
            vga_write(my_list[i].task);
            vga_write("\n");
            found = 1;
        }
    }
    if(!found) vga_write("No tasks found.\n");
}

void draw_menu_item(int id, int selected, const char* text) {
    if (id == selected) {
        // Highlighting logic
        vga_write(" > ");               // Arrow pointer
        vga_set_color(0x70);            // Invert: Black text on Light Gray background
        vga_write(text);
        vga_set_color(NOTEBOOK_YELLOW); // Reset to standard AliOS Yellow/Blue
    } else {
        vga_write("   ");               // Spacer for non-selected items
        vga_write(text);
    }
    vga_write("\n");
}
void cmd_menu(char* args) {
    int selected = 1;
    int total_options = 7; 
    int running = 1;

    vga_clear();

    while (running) {
        // Jump back to the top-left to overwrite, not scroll
        vga_set_cursor(0, 0); 

        // Header Section
        vga_set_color(0x1F); // White on Blue (Status Bar Style)
        vga_write("========================================\n");
        vga_write("          AliOS 4.0 - TOOLBOX           \n");
        vga_write("      (Use Arrows to Move, Enter)       \n");
        vga_write("========================================\n\n");
        vga_set_color(NOTEBOOK_YELLOW);

        // Draw all buttons
        draw_menu_item(1, selected, "[ 1. SYSTEM INFO (NEOFETCH) ]");
        draw_menu_item(2, selected, "[ 2. TO-DO LIST (TDSHW)     ]");
        draw_menu_item(3, selected, "[ 3. CALCULATOR (CALC)      ]");
        draw_menu_item(4, selected, "[ 4. QURAN AYAH             ]");
        draw_menu_item(5, selected, "[ 5. DRAW PLANE ART         ]");
        draw_menu_item(6, selected, "[ 6. LOCK SYSTEM            ]");
        draw_menu_item(7, selected, "[ 7. EXIT MENU              ]");
        
        vga_write("\n========================================\n");

        // Polling loop for Keyboard Input
        while (!(inb(0x64) & 0x01)) {
            vga_draw_status_bar(); // Keeps the clock ticking!
        }

        unsigned char scancode = inb(0x60);

        // Arrow and Input Logic
        if (scancode == 0x48) {        // UP ARROW
            if (selected > 1) selected--;
        } 
        else if (scancode == 0x50) {   // DOWN ARROW
            if (selected < total_options) selected++;
        } 
        else if (scancode == 0x1C) {   // ENTER KEY
            vga_clear(); 
            
            // saExecute the selected tool
            if (selected == 1) { cmd_neofetch(0); sleep_ms(1000); vga_clear(); }
            if (selected == 2) { todo_show(); sleep_ms(1000); vga_clear(); }
            if (selected == 3) { vga_write("Use it in terminal\n"); sleep_ms(1000); vga_clear(); }
            if (selected == 4) { cmd_ayah(); sleep_ms(1000); vga_clear(); }
            if (selected == 5) { draw_custom_plane(); sleep_ms(1000); vga_clear(); }
            if (selected == 6) { shell_lock(); sleep_ms(1000); vga_clear(); }
            if (selected == 7) { running = 0; }

            // If we didn't exit, wait for a key before returning to menu
            if (running && selected != 7) {
                // deleted bc of sleep and this shit doesnt work
                while (!(inb(0x64) & 0x01)); 
                inb(0x60); // Flush the buffer
            }
        }
        else if (scancode == 0x01) {   // ESC KEY
            running = 0;
        }
    }

    vga_clear();
    vga_write("Returned to AliOS Shell.\n> ");
}
void play_bad_apple() {
     unsigned short* vga_hardware = (unsigned short*)VGA_ADDRESS;
     for (int f = 0; f < APPLE_FRAME_COUNT; f++) {
         const char* frame = apple_frames[f];
         for (int i = 0; i < 1920; i++) { // Exactly 24 rows
             vga_hardware[i] = (unsigned short)frame[i] | (unsigned short)0x0F << 8;
         }
         // The Status Bar at row 24 is safe!
         if (f % 5 == 0) vga_draw_status_bar(); 
         sleep_ms(250);
     }
 }
void cmd_read_disk(char* args) {
	if (args == 0 || *args == '\0') {
		vga_write("Usage: read_sector [sector_num]\n");
		return;
	}
	uint32_t lba = (uint32_t)atoi_custom(args);
	uint8_t sector_buffer[512];
	ide_read_sector_bytes(lba, sector_buffer);
	vga_write("\nLBA Sector: ");
	vga_write(args);
	vga_write("\n------------------------------------------\n");
	for (int i = 0; i < 128; i++) {
		char hex_buf[4];
		itohex(sector_buffer[i], hex_buf);
		if (sector_buffer[i] < 0x10)
		vga_write("0");
		vga_write(hex_buf);
		vga_write(" ");
		if ((i + 1) % 16 == 0) {
			vga_write(" | ");
			for (int j = i - 15; j <= i; j++) {
				char c = (char)sector_buffer[j];
				if (c >= 32 && c <= 126)
				vga_putchar(c);
				else vga_putchar('.');
			}
			vga_write("\n");
		}
	}
	vga_write("------------------------------------------\n");
}
void cmd_write_disk(char* args) {
    // 1. Check for empty input
    if (args == 0 || *args == '\0') {
        vga_write("Usage: write_sector <lba> <string>\n");
        vga_write("Example: write_sector 10 Hello_AliOS\n");
        return;
    }

    
    char* lba_str = args;
    char* data_str = 0;

    
    for (int i = 0; args[i] != '\0'; i++) {
        if (args[i] == ' ') {
            args[i] = '\0';        // Null-terminate the LBA string
            data_str = &args[i+1]; // Point to the start of the data
            break;
        }
    }

    if (!data_str) {
        vga_write("Error: Missing data string.\n");
        return;
    }

    
    uint32_t lba = (uint32_t)atoi_custom(lba_str);

    
    static uint8_t sector_buffer[512];
    for (int i = 0; i < 512; i++) {
        sector_buffer[i] = 0;
    }

    
    int len = 0;
    while (data_str[len] != '\0' && len < 511) {
        sector_buffer[len] = (uint8_t)data_str[len];
        len++;
    }

    
    vga_write("AliOS Disk: Writing to LBA ");
    vga_write(lba_str); 
    vga_write("... ");

    ide_write_sector_bytes(lba, sector_buffer);

    vga_write("DONE!!\n");
}
uint32_t ide_calculate_usage_FULL() {
    uint8_t buffer[512];
    uint32_t used_count = 0;
    uint32_t total_sectors = ide_get_total_sectors();

    vga_write("Scanning disk... "); // Warning for the user

    for (uint32_t s = 0; s < total_sectors; s++) {
        ide_read_sector_bytes(s, buffer);
        
        // Check if sector is non-zero
        for (int i = 0; i < 512; i++) {
            if (buffer[i] != 0) {
                used_count++;
                break; 
            }
        }

        // Progress indicator every 1000 sectors so i know it's alive
        if (s % 1000 == 0) vga_putchar('.'); 
    }
    
    vga_write(" DONE\n");
    return used_count;
}

void print_disk_info(char* args) {
    uint32_t total_sectors = ide_get_total_sectors();
    if (total_sectors == 0) {
        vga_write("Error: No drive found.\n");
        return;
    }

    // WARNING: This will be slow on large disks!
    uint32_t used_sectors = ide_calculate_usage_FULL(); 
    uint32_t free_sectors = total_sectors - used_sectors;

    char unit = 'm'; 
    if (args && args[0] != '\0') unit = args[0];

    // Using 64-bit math to be safe
    uint64_t total_f, used_f, free_f;
    const char* label;

    if (unit == 'k') {
        total_f = (uint64_t)total_sectors / 2;
        used_f  = (uint64_t)used_sectors / 2;
        label   = " KB";
    } else if (unit == 'g') {
        total_f = (uint64_t)total_sectors / 2097152;
        used_f  = (uint64_t)used_sectors / 2097152;
        label   = " GB";
    } else {
        total_f = (uint64_t)total_sectors / 2048;
        used_f  = (uint64_t)used_sectors / 2048;
        label   = " MB";
    }

    // Floor protection: if sectors > 0, show at least 1 unit
    if (used_sectors > 0 && used_f == 0) used_f = 1;
    free_f = total_f - used_f;

    char b1[20], b2[20], b3[20];
    vga_write("\nTotal: "); vga_write(itoa((int)total_f, b1)); vga_write(label);
    vga_write("\nUsed:  "); vga_write(itoa((int)used_f, b2));  vga_write(label);
    vga_write("\nFree:  "); vga_write(itoa((int)free_f, b3));  vga_write(label);
    vga_write("\n");
}
void cmd_disk_wipe(char* args) {
    uint32_t total_sectors = ide_get_total_sectors();
    uint8_t zero_buffer[512];
    
    // Fill buffer with zeros once
    for (int i = 0; i < 512; i++) zero_buffer[i] = 0;

    vga_write("CRITICAL: Wiping Disk... ");

    for (uint32_t s = 0; s < total_sectors; s++) {
        ide_write_sector_bytes(s, zero_buffer);
        
        // Progress bar every 5%
        if (s % (total_sectors / 20) == 0) vga_write("#");
    }

    vga_write("\nDisk Erased Successfully.\n");
}

void seed_rand(uint32_t seed) {
    next_rand = seed;
}

uint8_t get_rand_byte() {
    // Simple LCG formula
    next_rand = next_rand * 1103515245 + 12345;
    return (uint8_t)(next_rand / 65536) % 256;
}

void cmd_disk_random(char* args) {
    uint32_t total_sectors = ide_get_total_sectors();
    uint8_t rand_buffer[512];
    
    // Seed the randomizer using current seconds
    seed_rand(cmos_get_sec());

    vga_write("Writing Random Noise... ");

    for (uint32_t s = 0; s < total_sectors; s++) {
        // Fill buffer with new random noise for every sector
        for (int i = 0; i < 512; i++) {
            rand_buffer[i] = get_rand_byte();
        }

        ide_write_sector_bytes(s, rand_buffer);

        if (s % (total_sectors / 20) == 0) vga_write("?");
    }

    vga_write("\nDisk Randomized.\n");
}


void cmd_disk_speed(char* args) {
    uint32_t total_sectors = ide_get_total_sectors();
    
    // We need at least 10MB of space to run a proper test
    if (total_sectors < 40000) {
        vga_write("Disk too small for reliable speed test.\n");
        return;
    }

    uint8_t buffer[512];
    // Fill buffer with a pattern so the disk actually has to work
    for(int i = 0; i < 512; i++) buffer[i] = (uint8_t)(i % 255);

    // Test 4MB (8192 sectors)
    uint32_t test_sectors = 8192; 
    uint32_t safe_offset = 20000; // Start 10MB into the disk
    
    vga_write("--- AliOS 4.0 I/O Benchmark (4MB Test) ---\n");
    vga_write("Target: LBA "); 
    char lba_buf[16]; vga_write(itoa(safe_offset, lba_buf));
    vga_write("\n\n");

    // --- WRITE TEST ---
    vga_write("Testing Write Speed... ");
    unsigned int start_w = get_uptime_ms(); 
    for (uint32_t s = 0; s < test_sectors; s++) {
        ide_write_sector_bytes(safe_offset + s, buffer);
        
        // Minor progress update every 1MB
        if (s % 2048 == 0 && s > 0) vga_putchar('#');
    }
    unsigned int time_w = get_uptime_ms() - start_w;
    vga_write(" Done.\n");

    // --- READ TEST ---
    vga_write("Testing Read Speed...  ");
    unsigned int start_r = get_uptime_ms();
    for (uint32_t s = 0; s < test_sectors; s++) {
        ide_read_sector_bytes(safe_offset + s, buffer);
        
        if (s % 2048 == 0 && s > 0) vga_putchar('#');
    }
    unsigned int time_r = get_uptime_ms() - start_r;
    vga_write(" Done.\n\n");

    // --- RESULTS CALCULATION ---
    char buf_w[16], buf_r[16], ms_buf[16];

    // Formula: (4MB * 1000) / time_in_ms = MB/s
    vga_write("RESULTS:\n");
    
    if (time_w > 0) {
        uint32_t speed_w = (4 * 1000) / time_w;
        vga_write("  Write: "); vga_write(itoa(speed_w, buf_w)); vga_write(" MB/s ");
        vga_write("("); vga_write(itoa(time_w, ms_buf)); vga_write(" ms)\n");
    } else {
        vga_write("  Write: Too fast to measure (< 1ms)\n");
    }

    if (time_r > 0) {
        uint32_t speed_r = (4 * 1000) / time_r;
        vga_write("  Read:  "); vga_write(itoa(speed_r, buf_r)); vga_write(" MB/s ");
        vga_write("("); vga_write(itoa(time_r, ms_buf)); vga_write(" ms)\n");
    } else {
        vga_write("  Read:  Too fast to measure (< 1ms)\n");
    }
    
    vga_write("\nNote: Speeds are limited by PIO Mode overhead.\n");
}

char* get_filename_arg(char* args) {
    if (args == 0 || *args == '\0') return 0;
    while (*args == ' ') args++; // Skip leading spaces
    if (*args == '\0') return 0;
    return args;
}

void kgets_multiline(char* buffer, int max_len) {
    int i = 0;
    vga_write("(Press ESC to save and exit)\n> ");

    while (i < max_len - 1) {
        char c = wait_for_key();

        // 27 is the ASCII/Scancode often used for ESC
        if (c == 27) { 
            buffer[i] = '\0';
            vga_write("\n[Saving...]\n");
            break;
        } 
        
        else if (c == '\n') {
            buffer[i++] = '\n';
            vga_putchar('\n');
            vga_write("> "); // Visual cue for new line
        } 
        
        else if (c == '\b') {
            if (i > 0) {
                if (buffer[i-1] == '\n') {
                    // Logic for backspacing a newline is tricky in VGA
                    // For now, let's just prevent backspacing past a newline
                    continue; 
                }
                i--;
                vga_putchar('\b');
            }
        } 
        
        else if (c >= 32 && c <= 126) {
            buffer[i++] = c;
            vga_putchar(c);
        }
    }
    buffer[i] = '\0';
}

void kgets(char* buffer, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        char c = wait_for_key();

        if (c == '\n') {
            buffer[i] = '\0';
            vga_putchar('\n');
            break;
        } else if (c == '\b') {
            if (i > 0) {
                i--;
                // Standard VGA backspace: move back, print space, move back
                vga_putchar('\b'); 
            }
        } else if (c >= 32 && c <= 126) { // Printable characters only
            buffer[i++] = c;
            vga_putchar(c);
        }
    }
    buffer[i] = '\0';
}
void cmd_edit(char* args) {
    char* filename = get_filename_arg(args);

    if (!filename || *filename == '\0') {
        vga_write("Usage: edit <filename>\n");
        return;
    }

    vga_write("\n--- AliOS 4 Multiline Notebook ---\n");
    vga_write("File: "); vga_write(filename); vga_write("\n");

    static char note_content[512] __attribute__((aligned(8)));
    for(int i = 0; i < 512; i++) note_content[i] = 0;

    kgets_multiline(note_content, 511);

    if (alifs_create(filename, note_content) == 0) {
        vga_write("Note saved to AliFS.\n");
    } else {
        vga_write("Error: Sector write failed.\n");
    }
}


void cmd_touch(char* args) {
    char* filename = get_filename_arg(args);

    if (!filename || *filename == '\0') {
        vga_write("Usage: touch <filename>\n");
        return;
    }

    if (alifs_create(filename, "") == 0) {
        vga_write("Created empty file: ");
        vga_write(filename);
        vga_write("\n");
    } else {
        vga_write("Error: Could not create file.\n");
    }
}
void cmd_cat(char* args) {
    char* filename = get_filename_arg(args);
    if (!filename || *filename == '\0') {
        vga_write("Usage: cat <filename>\n");
        return;
    }

    char* content = alifs_read(filename); 

    if (content) {
        vga_write("\n--- "); vga_write(filename); vga_write(" ---\n");
        vga_write(content);
        vga_write("\n------------------\n");
    } else {
        vga_write("Error: File not found.\n");
    }
}
void cmd_format(char* args) {
    vga_write("\n--- AliFS File System Format ---\n");
    vga_write("WARNING: This will clear the Inode table at LBA 20001.\n");
    vga_write("All files in the Notebook will be lost.\n");
    vga_write("Type 'CONFIRM' to proceed: ");

    char confirm[10];
    kgets(confirm, 9);

    if (strcmp(confirm, "CONFIRM") == 0) {
        vga_write("Formatting... ");
        alifs_format();
        vga_write("SUCCESS.\n");
    } else {
        vga_write("Format aborted.\n");
    }
}
void cmd_ls(char* args) {
    // alifs_list() handles the disk reading and VGA printing internally
    alifs_list();
}
void cmd_cd(char* args) {
    // If user types 'gtdi' with no args, or 'gtdi /', go back to root
    if (args == 0 || args[0] == '\0' || strcmp(args, "/") == 0) {
        strcpy(current_path, "/");
        vga_write("Returned to root.\n");
        return;
    }

    // Logic for moving into a sub-directory
    if (alifs_is_directory(args)) {
        if (strcmp(current_path, "/") == 0) {
            // If we are at root, the new path is just the folder name
            char temp[256];
            temp[0] = '/';
            strcpy(&temp[1], args);
            strcpy(current_path, temp);
        } else {
            // Append the new folder: /old/new
            int len = strlen(current_path);
            current_path[len] = '/';
            strcpy(&current_path[len+1], args);
        }
    } else {
        vga_write("Error: Directory not found.\n");
    }
}

void cmd_mkdir(char* args) {
    char* dirname = get_filename_arg(args);
    if (!dirname) {
        vga_write("Usage: mkdir <name>\n");
        return;
    }

    if (alifs_mkdir(dirname) == 0) {
        vga_write("Directory created.\n");
    } else {
        vga_write("Error: Could not create directory.\n");
    }
}

void aosdcserver() {
    vga_write("https://discord.gg/ymxpjGq9Gu");
}
void cmd_asma(char* args) {
    name_99_t names[] = {
        {"Ar-Rahman", "The Beneficent", "He who wills goodness and mercy for all His creatures."},
        {"Ar-Rahim", "The Merciful", "He who acts with extreme kindness."},
        {"Al-Malik", "The Eternal Lord", "The Sovereign Lord, The One with complete Dominion."},
        {"Al-Quddus", "The Most Sacred", "The One who is pure from any imperfection."},
        {"As-Salam", "The Embodiment of Peace", "The One who frees His servants from all danger."},
        {"Al-Mu'min", "The Infuser of Faith", "The One who witnessed for Himself and whose help is explained."},
        {"Al-Muhaymin", "The Preserver of Safety", "The One who witnesses the evolution of His creatures."},
        {"Al-Aziz", "The Mighty One", "The Victorious One where no resistance can be raised."},
        {"Al-Jabbar", "The Omnipotent One", "The Irresistible Subduer."},
        {"Al-Mutakabbir", "The Dominant One", "The One who is proud and beyond every creation."}
    };

    int db_size = sizeof(names) / sizeof(name_99_t);
    
    // Seed the randomness using your CMOS and Uptime
    int r = (cmos_get_sec() + (get_uptime_ms() % 100)) % db_size;

    vga_write("\n--- [ ASMA-UL-HUSNA ] ---\n");
    vga_write("Name: "); vga_write(names[r].ar);
    vga_write(" ("); vga_write(names[r].en); vga_write(")\n");
    vga_write("Meaning: "); vga_write(names[r].meaning);
    vga_write("\n-------------------------\n");
}
void install_aos() {
  cmd_install_os();
}
void cmd_divbyzero(char* args) {
    volatile int x = 0;
    volatile int y = 5; 
    
    vga_write("\n  [!] EXECUTING ILLEGAL INSTRUCTION: DIVIDE BY ZERO\n");    
    volatile int result = y / x; 
    
    (void)result; 
}
void gui() {
  cmd_start_gui();
}
/* Helper to print the full 0-F color lookup table */
void print_color_table() {
    vga_write(" Hex | Color Name         | Hex | Color Name\n");
    vga_write("-----+--------------------+-----+--------------------\n");
    vga_write("  0  | Black              |  8  | Dark Gray\n");
    vga_write("  1  | Blue               |  9  | Light Blue\n");
    vga_write("  2  | Green              |  A  | Light Green\n");
    vga_write("  3  | Cyan               |  B  | Light Cyan\n");
    vga_write("  4  | Red                |  C  | Light Red\n");
    vga_write("  5  | Magenta            |  D  | Light Magenta\n");
    vga_write("  6  | Brown              |  E  | Yellow\n");
    vga_write("  7  | Light Gray         |  F  | Bright White\n");
}

/* Helper to convert a single character ('0'-'F') into its integer hex value */
int parse_single_hex_char(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // Invalid hex character
}

/* The interactive color wizard command */
void cmd_color(char* args) {
    (void)args; // Ignore any string arguments typed after the command
    
    char input_buf[10];
    int fg = -1;
    int bg = -1;

    // --- STEP 1: FOREGROUND (TEXT) SELECTION ---
    print_color_table();
    vga_write("\nWhat color do you want the text to be? (Enter 0-F): ");
    
    kgets(input_buf, 9);
    fg = parse_single_hex_char(input_buf[0]);

    if (fg < 0 || fg > 15) {
        vga_write("Error: Invalid color selection code. Aborting.\n");
        return;
    }

    vga_write("\n");

    // --- STEP 2: BACKGROUND SELECTION ---
    print_color_table();
    vga_write("\nWhat color do you want the background to be? (Enter 0-F): ");
    
    kgets(input_buf, 9);
    bg = parse_single_hex_char(input_buf[0]);

    if (bg < 0 || bg > 15) {
        vga_write("Error: Invalid color selection code. Aborting.\n");
        return;
    }

    // --- STEP 3: BUILD AND SET ATTRIBUTE BYTE ---
    // Shift background to high nibble, leave foreground in low nibble
    unsigned char final_attribute = (unsigned char)((bg << 4) | fg);

    vga_set_color(final_attribute);

    vga_write("\nMatrix updated successfully!\n");
}

/* --- Shell Logic --- */
void shell_register_command(const char* name, const char* desc, command_func func) {
    command_node_t* new_node = (command_node_t*)kmalloc(sizeof(command_node_t));
    
    int i = 0;
    while(name[i] && i < 31) { new_node->name[i] = name[i]; i++; }
    new_node->name[i] = '\0';

    i = 0;
    while(desc[i] && i < 63) { new_node->description[i] = desc[i]; i++; }
    new_node->description[i] = '\0';

    new_node->function = func;
    new_node->next = command_list;
    command_list = new_node;
}


void shell_init() {
    shell_register_command("help", "List all available commands", cmd_help);
    shell_register_command("cls",  "Clear the notebook screen",   cmd_cls);
    shell_register_command("echo", "Print text to the screen",    cmd_echo);
    shell_register_command("neofetch", "Display dynamic system info", cmd_neofetch);
    shell_register_command("uptime", "Show how long AliOS has been running", cmd_uptime);
    shell_register_command("free", "Check dynamic RAM usage", cmd_free);
    shell_register_command("timezone", "Adjust the status bar clock offset", shell_cmd_timezone);
    shell_register_command("lock", "Locks the system", shell_lock);
    shell_register_command("test",     "Verify timer calibration",    cmd_test);
    shell_register_command("beep", "Play a system alert sound", cmd_beep);
    shell_register_command("about_dev", "About Dev", cmd_about_dev);
    shell_register_command("plane", "Show a art of a plane", draw_custom_plane);
    shell_register_command("twins", "Shows my twins names", twins);
    shell_register_command("sleep", "Sleep", sys_sleep);
    shell_register_command("calc", "Calculator", command_calc);
    shell_register_command("peek", "Inspect raw memory addresses", cmd_peek);
    shell_register_command("poke", "Write to memory addrs", cmd_poke);
    shell_register_command("run", "Execute AliScript code", cmd_run_script);
    shell_register_command("ayah", "Choose Random Quran Ayah and Print it (im turning into terry davis)", cmd_ayah);
    shell_register_command("verse", "Choose Random Bible Verse and Print it", cmd_verse);
    shell_register_command("set", "Set VAR", cmd_set);
    shell_register_command("get", "Get VAR", cmd_get);
    shell_register_command("tdadd", "Add to ToDo List", todo_add);
    shell_register_command("tdshw", "Show ToDo List", todo_show);
    shell_register_command("menu", "AliOS Menu", cmd_menu);
    shell_register_command("badapple", "Bad Apple", play_bad_apple);
    shell_register_command("read_sector", "Read Sector IDE", cmd_read_disk);
    shell_register_command("write_sector", "Write Sector IDE", cmd_write_disk);
    shell_register_command("sfree", "Storage info: df [k|m|g]", print_disk_info);
    shell_register_command("dwipe", "Erase the whole disk (zero out)", cmd_disk_wipe);
    shell_register_command("dshred", "Fill the disk with random noise", cmd_disk_random);
    shell_register_command("rwsp", "R/W Disk Speed Test", cmd_disk_speed);
    shell_register_command("lidi", "List files on AliFS", cmd_ls);   
    shell_register_command("crfi", "Create a new empty file", cmd_touch);
    shell_register_command("editfi", "Write text to a file", cmd_edit);
    shell_register_command("refi", "Read file content", cmd_cat);
    shell_register_command("fmrt","Wipe and init AliFS", cmd_format);
    shell_register_command("mkdir", "Create a new directory", cmd_mkdir);
    shell_register_command("gtdi", "Go To DIrectory", cmd_cd);
    shell_register_command("aosdcserv", "AliOS Discord Server", aosdcserver);
    shell_register_command("asma", "Random Name of Allah and its meaning", cmd_asma);
    shell_register_command("install", "Install AliOS", install_aos);
    shell_register_command("divbyzero", "DivbyZero", cmd_divbyzero);
    shell_register_command("gui", "GUI", gui);
    shell_register_command("color", "Interactive text and background color customization wizard", cmd_color);
}

void shell_dispatch(char* buffer) {
    // If the user just hits enter, just print a new prompt on a new line
    if (strlen(buffer) == 0) {
        vga_write("\nAliOS:");
        vga_write(current_path);
        vga_write("> ");
        return;
    }

    char* args = 0;
    for (int i = 0; buffer[i]; i++) {
        if (buffer[i] == ' ') {
            buffer[i] = '\0';
            args = &buffer[i+1];
            break;
        }
    }

    command_node_t* curr = command_list;
    while (curr) {
        if (strcmp(curr->name, buffer) == 0) {
            vga_write("\n"); // Move to new line before command output
            curr->function(args);
            vga_write("\nAliOS:");
            vga_write(current_path);
            vga_write("> ");
            return;
        }
        curr = curr->next;
    }

    // If command not found
    vga_write("\nAliOS: '");
    vga_write(buffer);
    vga_write("' not found. Type 'help'.\n ");
    vga_write("\nAliOS:");
    vga_write(current_path);
    vga_write("> ");
}
void shell_tab_complete(char* buffer, int* len) {
    command_node_t* curr = command_list;
    while (curr) {
        if (strncmp(curr->name, buffer, *len) == 0) {
            char* rest = curr->name + *len;
            vga_write(rest);
            while (*rest) buffer[(*len)++] = *rest++;
            buffer[*len] = '\0';
            return;
        }
        curr = curr->next;
    }
}
