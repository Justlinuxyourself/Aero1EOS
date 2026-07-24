
global load_tss
load_tss:
    ; Load the Task Register (TR)
    mov ax, 0x38  ; 0x38 is our TSS index in GDT
    ltr ax
    ret
