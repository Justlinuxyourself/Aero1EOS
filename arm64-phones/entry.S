.section ".text.boot"
.global _start

_start:
    // Safe guard: Read core multi-processor affinity register
    mrs     x0, mpidr_el1
    and     x0, x0, #0xFF        // Isolate Core ID (Affinity 0)
    cbz     x0, primary_core     // If 0, execute kernel_main

secondary_core_hold:
    wfe
    b       secondary_core_hold

primary_core:
    // Mask out system interrupts
    msr     daifset, #0xf

    // Initialize .bss boundaries to clean memory state
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
    sub     x1, x1, x0
    cbz     x1, init_stack

clear_bss:
    str     xzr, [x0], #8
    subs    x1, x1, #8
    b.gt    clear_bss

init_stack:
    // Anchor the top of the stack frame within the safe memory space
    ldr     x0, =_kernel_end
    add     x0, x0, #0x40000     // Allocate 256KB for safe workspace runtime
    mov     sp, x0

    // Direct jump into operating system kernel entry point
    bl      kernel_main

kernel_panic_halt:
    wfe
    b       kernel_panic_halt

