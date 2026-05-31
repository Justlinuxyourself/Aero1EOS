int vfs_open(char* path);
int vfs_read(int fd, uint8_t* buffer, int count);
int vfs_write(int fd, uint8_t* buffer, int count);

void syscall_handler(registers_t *regs) {
    switch (regs->rax) {
        case SYS_WRITE:
            vfs_write((int)regs->rbx, (uint8_t*)regs->rcx, (int)regs->rdx);
            break;
        case SYS_OPEN:
            regs->rax = vfs_open((char*)regs->rbx);
            break;
        case SYS_READ:
            regs->rax = vfs_read((int)regs->rbx, (uint8_t*)regs->rcx, (int)regs->rdx);
            break;
    }
}
