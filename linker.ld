ENTRY(_start)

SECTIONS {
    /* Standard load address for Multiboot kernels */
    . = 1M;

		_kernel_end = .;


    /* The Multiboot header must be near the start of the file */
    .multiboot : ALIGN(8)
    {
        KEEP(*(.multiboot))
    }

    .text : ALIGN(4K)
    {
        *(.text)
    }

    .rodata : ALIGN(4K) { *(.rodata) }
    .data   : ALIGN(4K) { *(.data) }
    .bss    : ALIGN(4K) { *(.bss) *(COMMON) }

    _kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}
