extern syscall_handler
global isr_syscall
isr_syscall:
    ; 1. CPU has pushed: SS, RSP, RFLAGS, CS, RIP
    
    ; 2. Push all scratch/preserved registers to save state
    push rbp
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

    ; 3. Pass the pointer to the registers_t struct (currently on the stack)
    mov rdi, rsp           
    call syscall_handler

    ; 5. Pop all registers in REVERSE order
    pop rax         ; This now contains the return value from vfs_open
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
    
    ; 6. Stack is now back to: [SS, RSP, RFLAGS, CS, RIP]
    iretq
