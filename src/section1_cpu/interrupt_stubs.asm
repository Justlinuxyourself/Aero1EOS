[bits 64]

global exception_table
extern c_kernel_panic
extern timer_handler
section .text

; --- The Magic Assembly Macro ---
; Some exceptions push an error code onto the stack automatically, some don't.
; This macro ensures the stack layout looks identical for every single one.
%macro EXCEPTION_NO_ERR 1
global exception_stub_%1
exception_stub_%1:
    push qword 0         ; Push a dummy error code since CPU didn't provide one
    push qword %1        ; Push the Exception ID number
    jmp exception_common_route
%endmacro

%macro EXCEPTION_WITH_ERR 1
global exception_stub_%1
exception_stub_%1:
    push qword %1        ; Push the Exception ID number (Error code is already on stack)
    jmp exception_common_route
%endmacro

; --- Define All 32 Architectural Exceptions By Hand ---
EXCEPTION_NO_ERR   0   ; Divide-by-Zero
EXCEPTION_NO_ERR   1   ; Debug
EXCEPTION_NO_ERR   2   ; Non-Maskable Interrupt
EXCEPTION_NO_ERR   3   ; Breakpoint
EXCEPTION_NO_ERR   4   ; Overflow
EXCEPTION_NO_ERR   5   ; Bound Range Exceeded
EXCEPTION_NO_ERR   6   ; Invalid Opcode (Exec garbage memory)
EXCEPTION_NO_ERR   7   ; Device Not Available
EXCEPTION_WITH_ERR 8   ; Double Fault (Exception during exception)
EXCEPTION_NO_ERR   9   ; Coprocessor Segment Overrun
EXCEPTION_WITH_ERR 10  ; Invalid TSS
EXCEPTION_WITH_ERR 11  ; Segment Not Present
EXCEPTION_WITH_ERR 12  ; Stack-Segment Fault
EXCEPTION_WITH_ERR 13  ; General Protection Fault (GPF)
EXCEPTION_WITH_ERR 14  ; Page Fault (Bad memory dereference)
EXCEPTION_NO_ERR   15  ; Reserved
EXCEPTION_NO_ERR   16  ; x87 Floating-Point Exception
EXCEPTION_WITH_ERR 17  ; Alignment Check
EXCEPTION_NO_ERR   18  ; Machine Check
EXCEPTION_NO_ERR   19  ; SIMD Floating-Point
EXCEPTION_NO_ERR   20  ; Virtualization Exception
EXCEPTION_WITH_ERR 21  ; Control Protection Exception
EXCEPTION_NO_ERR   22  ; Reserved
EXCEPTION_NO_ERR   23  ; Reserved
EXCEPTION_NO_ERR   24  ; Reserved
EXCEPTION_NO_ERR   25  ; Reserved
EXCEPTION_NO_ERR   26  ; Reserved
EXCEPTION_NO_ERR   27  ; Reserved
EXCEPTION_NO_ERR   28  ; Hypervisor Injection Exception
EXCEPTION_WITH_ERR 29  ; VMM Communication Exception
EXCEPTION_WITH_ERR 30  ; Security Exception
EXCEPTION_NO_ERR   31  ; Reserved

; --- Common Vector Engine ---
exception_common_route:
    ; Push all general purpose registers to preserve the machine's exact state
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    ; Save vital memory/paging debugging contexts
    mov rax, cr2
    push rax             ; CR2 holds the address that caused a page fault
    mov rax, cr3
    push rax             ; CR3 holds the active page directory pointer

    ; Pass the address of this stack frame as the 1st argument (RDI) to C
    mov rdi, rsp
    
    cld                  ; Clear direction flag for standard C calling conventions
    call c_kernel_panic

.dead_halt:
    cli
    hlt
    jmp .dead_halt

global timer_isr_stub
timer_isr_stub:
    push rax            ; Save RAX
    push rbx            ; Save RBX
    
    call timer_handler  ; Call C function
    
    mov al, 0x20        ; End of Interrupt (EOI)
    out 0x20, al
    
    pop rbx
    pop rax
    iretq               ; Mandatory return

section .rodata
align 8
exception_table:
%assign i 0
%rep 32
    dq exception_stub_%[i]
%assign i i+1
%endrep
    dq timer_isr_stub   