#include <stdint.h>

extern void vga_set_color(unsigned char color);
extern void vga_write(const char* data);
extern void vga_putchar(char c);

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

// IDT Pointer structure for lidt instruction
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
        idt[i].selector         = 0x18;
        idt[i].ist              = 0;
        idt[i].type_attributes  = 0x8E;
        idt[i].offset_mid       = (uint16_t)((addr >> 16) & 0xFFFF);
        idt[i].offset_high      = (uint32_t)((addr >> 32) & 0xFFFFFFFF);
        idt[i].reserved         = 0;
    }

    // Call native assembly instruction to bind this framework table to CPU
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

void c_kernel_panic(CpuPanicState* state) {
    vga_set_color(0x0C);
    for (int i = 0; i < 80 * 25; i++) vga_putchar(' ');

    vga_write("================================================================================\n");
    vga_write("                   !!! CRITICAL KERNEL PANIC: ALIOS 4.0 !!!                     \n");
    vga_write("================================================================================\n\n");
    
    vga_write(" CPU TRAPPED EXCEPTION: ");
    if (state->exception_vector < 32) {
        vga_write(exception_messages[state->exception_vector]);
    } else {
        vga_write("Unknown System Hardware Glitch");
    }
    vga_write("\n\n");
    
    vga_write("  Instruction Pointer (RIP):  "); print_hex64(state->rip); vga_write("\n");
    vga_write("  Hardware Error Code:        "); print_hex64(state->hardware_error_code); vga_write("\n");
    vga_write("  Fault Address State (CR2): "); print_hex64(state->cr2); vga_write("\n");
    vga_write("  Page Directory Base (CR3): "); print_hex64(state->cr3); vga_write("\n\n");
    
    vga_write(" Registers Context Raw Dump:\n");
    vga_write("  RAX: "); print_hex64(state->rax); vga_write("  RBX: "); print_hex64(state->rbx); vga_write("  RCX: "); print_hex64(state->rcx); vga_write("\n");
    vga_write("  RDX: "); print_hex64(state->rdx); vga_write("  RSI: "); print_hex64(state->rsi); vga_write("  RDI: "); print_hex64(state->rdi); vga_write("\n");
    vga_write("  RBP: "); print_hex64(state->rbp); vga_write("  R8:  "); print_hex64(state->r8);  vga_write("  R9:  "); print_hex64(state->r9);  vga_write("\n");
    vga_write("  R10: "); print_hex64(state->r10); vga_write("  R11: "); print_hex64(state->r11); vga_write("  R12: "); print_hex64(state->r12); vga_write("\n");
    vga_write("================================================================================");
}
