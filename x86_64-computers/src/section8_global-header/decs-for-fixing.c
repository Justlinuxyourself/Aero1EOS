#include "global.h"

static inline void cpuid(int code, unsigned int* a, unsigned int* b, unsigned int* c, unsigned int* d) {
    __asm__ volatile ("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(code));
}

void get_cpu_name(char* name) {
    unsigned int eax, ebx, ecx, edx;
    uint32_t* ptr = (uint32_t*)name;

    for (uint32_t i = 0; i < 3; i++) {
        cpuid(0x80000002 + i, &eax, &ebx, &ecx, &edx);
        ptr[i * 4 + 0] = eax;
        ptr[i * 4 + 1] = ebx;
        ptr[i * 4 + 2] = ecx;
        ptr[i * 4 + 3] = edx;
    }
    name[48] = '\0'; // Ensure null-termination
}
