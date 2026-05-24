#include <stdint.h>

extern void vga_set_color(unsigned char color);
extern void vga_write(const char* data);
extern void vga_putchar(char c);
extern void update_hardware_cursor(int pos);
extern void play_sound(unsigned int nFrequence);

// Independent TTY structure matching vga.c exactly
typedef struct {
    unsigned short* buffer;
    int cursor_pos;
    char command_buffer[80];
    int buffer_idx;
} tty_t;

extern tty_t ttys[];
extern int current_tty;

// 64-bit IDT Entry structure layout
typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attributes;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed)) IdtEntry;

// IDT Pointer structure for lids instruction
typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) IdtPointer;

typedef struct {
    uint64_t cr3;
    uint64_t cr2;
    uint64_t rax; uint64_t rbx; uint64_t rcx; uint64_t rdx;
    uint64_t rsi; uint64_t rdi; uint64_t rbp;
    uint64_t r8;  uint64_t r9;  uint64_t r10; uint64_t r11;
    uint64_t r12; uint64_t r13; uint64_t r14; uint64_t r15;
    
    // Tracked vectors
    uint64_t exception_vector;
    uint64_t hardware_error_code;
    uint64_t rip; // Address where execution broke
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) CpuPanicState;

static IdtEntry idt[32];
static IdtPointer idt_ptr;

extern uint64_t exception_table[32];

// Human-readable names for every exception context
static const char* exception_messages[] = {
    "0x00 Divide-by-Zero Error", "0x01 Debug Exception", "0x02 Non-Maskable Interrupt",
    "0x03 Breakpoint Exception", "0x04 Overflow Detected", "0x05 Bound Range Exceeded",
    "0x06 Invalid Opcode (Attempted to execute invalid instruction)", "0x07 Device Not Available",
    "0x08 Double Fault (Kernel trapped inside a fault handling cycle)", "0x09 Coprocessor Segment Overrun",
    "0x0A Invalid TSS Reference", "0x0B Segment Not Present", "0x0C Stack-Segment Fault",
    "0x0D General Protection Fault (GPF - Illegal Memory Access)", "0x0E Page Fault (Unmapped Virtual Addressing)",
    "0x0F Reserved/Unknown Entry Exception", "0x10 x87 Floating-Point Exception", "0x11 Alignment Check Failure",
    "0x12 Machine Check Exception", "0x13 SIMD Floating-Point Exception", "0x14 Virtualization Exception",
    "0x15 Control Protection Fault", "0x16 Reserved", "0x17 Reserved", "0x18 Reserved", "0x19 Reserved",
    "0x1A Reserved", "0x1B Reserved", "0x1C Hypervisor Injection Exception", "0x1D VMM Communication Exception",
    "0x1E Security Exception Modification Fault", "0x1F Reserved"
};

static void print_hex64(uint64_t value) {
    char hex_chars[] = "0123456789ABCDEF";
    vga_write("0x");
    for (int i = 60; i >= 0; i -= 4) {
        vga_putchar(hex_chars[(value >> i) & 0x0F]);
    }
}

void init_idt() {
    // Set limit and base for the 32 system exception entry registers
    idt_ptr.limit = (sizeof(IdtEntry) * 32) - 1;
    idt_ptr.base = (uint64_t)&idt;

    for (int i = 0; i < 32; i++) {
        uint64_t addr = exception_table[i];
        idt[i].offset_low       = (uint16_t)(addr & 0xFFFF);
        idt[i].selector         = 0x18; // Fits 64-bit selector from boot.asm
        idt[i].ist              = 0;
        idt[i].type_attributes  = 0x8E;
        idt[i].offset_mid       = (uint16_t)((addr >> 16) & 0xFFFF);
        idt[i].offset_high      = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
        idt[i].reserved         = 0;
    }
    
    // Call native assembly instruction to bind this framework table to CPU
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}
void kernel_panic_sound() {
    // Slide frequency down from 2000Hz to 100Hz
    for (int freq = 2000; freq > 100; freq -= 5) {
        play_sound(freq);
        for(volatile int i = 0; i < 200000; i++); // Fast delay for smooth sliding
    }
    
    // Hold a flatline low drone for a moment
    play_sound(80); 
    for(volatile int i = 0; i < 50000000; i++);
    
    // Kill the sound and halt the CPU completely
    nosound();
    while(1) {
        __asm__ volatile("cli; hlt"); 
    }
}

void c_kernel_panic(CpuPanicState* state) {
    // 1. Force color state variable to Crimson Red on Black (0x0C)
    vga_set_color(0x0C);

    // 2. Clear out tracking cursor to prevent vga_write out-of-bounds corruption
    ttys[current_tty].cursor_pos = 0;

    // 3. HARDWARE RESET: Direct blast 0xB8000 video card memory to pure black base
    volatile uint16_t* raw_vga_mem = (volatile uint16_t*)0xB8000;
    uint16_t scary_cell = (0x0C << 8) | ' ';
    
    for (int i = 0; i < 80 * 25; i++) {
        raw_vga_mem[i] = scary_cell;
        ttys[current_tty].buffer[i] = scary_cell; // Clear virtual TTY frame buffer too
    }

    // 4. Position the blinking hardware cursor completely off screen
    update_hardware_cursor(2000);

    // 5. Output scary industrial register telemetry dump
    vga_write("################################################################################\n");
    vga_write("                  CRITICAL HARDWARE FAULT DETECTED: AERO1EOS 4.0                  \n");
    vga_write("################################################################################\n\n");
    
    vga_write("  [!!] SYSTEM ENGINE TRAPPED EXCEPTION: ");
    if (state->exception_vector < 32) {
        vga_write(exception_messages[state->exception_vector]);
    } else {
        vga_write("UNKNOWN UNSTABLE HARDWARE GLITCH");
    }
    vga_write("\n\n");
    
    vga_write("  PROCESSOR HALTED. CODE EXECUTION TERMINATED.\n");
    vga_write("  --------------------------------------------------\n");
    vga_write("  FAULT LOCATION (RIP):   "); print_hex64(state->rip); vga_write("\n");
    vga_write("  HARDWARE ERROR CODE:    "); print_hex64(state->hardware_error_code); vga_write("\n");
    vga_write("  PAGE FAULT ADDR (CR2):  "); print_hex64(state->cr2); vga_write("\n");
    vga_write("  PAGE DIRECTORY (CR3):   "); print_hex64(state->cr3); vga_write("\n\n");
    
    vga_write("  REGISTER STATE DUMP:\n");
    vga_write("  RAX: "); print_hex64(state->rax); vga_write("  RBX: "); print_hex64(state->rbx); vga_write("  RCX: "); print_hex64(state->rcx); vga_write("\n");
    vga_write("  RDX: "); print_hex64(state->rdx); vga_write("  RSI: "); print_hex64(state->rsi); vga_write("  RDI: "); print_hex64(state->rdi); vga_write("\n");
    vga_write("  RBP: "); print_hex64(state->rbp); vga_write("  R8:  "); print_hex64(state->r8);  vga_write("  R9:  "); print_hex64(state->r9);  vga_write("\n");
    vga_write("  R10: "); print_hex64(state->r10); vga_write("  R11: "); print_hex64(state->r11); vga_write("  R12: "); print_hex64(state->r12); vga_write("\n");
    vga_write("################################################################################");
    vga_write("(hahaha looks whos cpu is fucked)");
    kernel_panic_sound();
    // Left vga_draw_status_bar() out entirely to keep it pristine full black!
}
