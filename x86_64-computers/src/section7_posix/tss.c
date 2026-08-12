#include "../section8_global-header/global.h"
#include <stdint.h>
#include <stddef.h>



static tss_t tss __attribute__((aligned(16)));

void setup_tss(uint64_t kernel_stack) {
    uint8_t* ptr = (uint8_t*)&tss;
    for (size_t i = 0; i < sizeof(tss_t); i++) ptr[i] = 0;
    tss.rsp0 = kernel_stack;
    tss.iopb_offset = sizeof(tss_t);
}

void patch_gdt_tss() {
    uint64_t base = (uint64_t)&tss;
    tss_base_low   = (uint16_t)(base & 0xFFFF);
    tss_base_mid   = (uint8_t)((base >> 16) & 0xFF);
    tss_base_high  = (uint8_t)((base >> 24) & 0xFF);
    tss_base_upper = (uint32_t)(base >> 32);
}
