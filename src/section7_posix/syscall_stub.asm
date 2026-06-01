extern syscall_handler

global isr_syscall
isr_syscall:
    ; CPU already pushed: SS, RSP, RFLAGS, CS, RIP
    push rbp
    
    ; Push callee-saved and scratch registers
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax
    
    ; Pass pointer to registers_t struct to the C handler
    mov rdi, rsp           
    call syscall_handler
    
    ; Pop registers in EXACT REVERSE order of the pushes
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
    pop rbp
    
    ; Return using the hardware-pushed frame
    iretq
