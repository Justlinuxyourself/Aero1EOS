.section ".text.boot"
.global _start

_start:
    // 1. Standard ARM zImage magic header spoofing
    b       real_start           // First instruction: jump over the header
    .word   0                    // Reserved
    .word   0                    // Reserved
    .word   0                    // Reserved
    .word   0                    // Reserved
    .word   0                    // Reserved
    .word   0                    // Reserved
    .word   0x016f2818           // Magic number (tells bootloader "I am a 32-bit ARM Linux kernel")
    .word   _start               // Start address
    .word   _kernel_end          // End address

real_start:
    // Mask out system interrupts
    cpsid   if

    // 2. Safe guard: Read core multi-processor affinity register
    mrc     p15, 0, r0, c0, c0, 5   
    and     r0, r0, #0xFF           
    cmp     r0, #0                  
    beq     primary_core            

secondary_core_hold:
    wfe
    b       secondary_core_hold

primary_core:
    // Initialize .bss boundaries
    ldr     r0, =__bss_start
    ldr     r1, =__bss_end
    cmp     r1, r0                  
    beq     init_stack
    mov     r2, #0                  

clear_bss:
    str     r2, [r0], #4            
    cmp     r0, r1                  
    blo     clear_bss               

init_stack:
    ldr     r0, =_kernel_end
    add     r0, r0, #0x40000        
    mov     sp, r0

    bl      kernel_main

kernel_panic_halt:
    wfe
    b       kernel_panic_halt

