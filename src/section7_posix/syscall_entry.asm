[bits 64]
global syscall_entry
extern c_syscall_handler

syscall_entry:
    swapgs                  ; Switch to kernel GS
    mov [gs:0], rsp         ; Save User Stack Pointer
    mov rsp, [gs:8]         ; Load Kernel Stack Pointer
    
    push rcx                ; SYSCALL uses RCX for RIP
    push r11                ; SYSCALL uses R11 for RFLAGS
    
    call c_syscall_handler  ; RDI, RSI, RDX are args
    
    pop r11
    pop rcx
    mov rsp, [gs:0]         ; Restore User RSP
    swapgs
    sysretq
