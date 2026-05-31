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

extern uint16_t tss_base_low;
extern uint8_t  tss_base_mid;
extern uint8_t  tss_base_high;
extern uint32_t tss_base_upper;

void patch_gdt_tss() {
    uint64_t base = (uint64_t)&tss;

    tss_base_low   = (uint16_t)(base & 0xFFFF);
    tss_base_mid   = (uint8_t)((base >> 16) & 0xFF);
    tss_base_high  = (uint8_t)((base >> 24) & 0xFF);
    tss_base_upper = (uint32_t)(base >> 32);
}

