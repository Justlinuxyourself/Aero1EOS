#include "../section8_global-header/global.h"
/*
Copyright (c) 2026 Ali  
All rights reserved.
*/
#include "heap.h"

// Pull the '_kernel_end' address from the linker script

// Start the heap exactly where the kernel finishes
static block_header_t* free_list_start = (block_header_t*)&_kernel_end;
static size_t total_allocated = 0;

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

void init_heap() {
    free_list_start->size = 0;
    free_list_start->is_free = 0;
    free_list_start->next = NULL;
}

void* kmalloc(size_t size) {
    size = ALIGN(size);
    block_header_t* current = free_list_start;

    // Search for a free block (First Fit)
    while (current->next != NULL) {
        if (current->is_free && current->size >= size) {
            current->is_free = 0;
            total_allocated += current->size;
            return (void*)(current + 1);
        }
        current = current->next;
    }

    // Allocate new block at the end
    block_header_t* new_block = (block_header_t*)((unsigned char*)current + sizeof(block_header_t) + current->size);
    
    new_block->size = size;
    new_block->is_free = 0;
    new_block->next = NULL;
    
    current->next = new_block;
    total_allocated += size;

    return (void*)(new_block + 1); 
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_header_t* block = (block_header_t*)ptr - 1;
    block->is_free = 1;
    total_allocated -= block->size;
}

size_t get_heap_usage() {
    return total_allocated;
}
