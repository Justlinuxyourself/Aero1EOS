#ifndef _POSIX_SYSCALL_H
#define _POSIX_SYSCALL_H

#include <stdint.h>

#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_READ  3
#define SYS_EXIT  4

typedef struct {
    // 1. Pushed by isr_syscall:
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, r8, r9, r10, r11, r12, r13, r14, r15, rbp;
    
    // 2. Pushed by the CPU hardware on 'int 0x80':
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed)) registers_t;


void init_syscalls();
void syscall_handler(registers_t *regs);

#endif
