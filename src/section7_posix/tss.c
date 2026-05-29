#include <stdint.h>

typedef struct {
    uint32_t reserved0;
    uint64_t rsp0; // Kernel stack used when ring 3 -> ring 0 happens
    uint64_t rsp1, rsp2;
    uint64_t reserved1;
    uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed)) tss_t;

static tss_t tss;

void setup_tss(uint64_t kernel_stack) {
    tss.rsp0 = kernel_stack;
    tss.iopb_offset = sizeof(tss_t);
}
