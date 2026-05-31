#ifndef _POSIX_SYSCALL_H
#define _POSIX_SYSCALL_H

#include <stdint.h>

#define SYS_WRITE 1
#define SYS_OPEN  2
#define SYS_READ  3
#define SYS_EXIT  4

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} registers_t;

void init_syscalls();
void syscall_handler(registers_t *regs);

#endif
