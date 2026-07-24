ENTRY(_start)

SECTIONS
{
    /* Common entry point address offset passed by Android boot image specs */
    . = 0x4008000;

    .text : {
        KEEP(*(.text.boot))
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss : ALIGN(16) {
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
        __bss_end = .;
    }

    _kernel_end = .;
}

