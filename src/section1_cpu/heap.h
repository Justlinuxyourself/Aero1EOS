/* 
Copyright (c) 2026 Ali  
All rights reserved.
*/
/* 
Copyright (c) 2026 Ali  
All rights reserved.
*/
#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>


typedef struct block_header {
    size_t size;            // Size of the data block (excluding header)
    int is_free;            // 1 if the block is available for reuse, 0 if used
    struct block_header* next; // Pointer to the next block in the linked list
} block_header_t;

/**
 * Initializes the heap at the address of the _kernel_end symbol.
 */
void init_heap();

/**
 * Allocates a block of memory of the specified size.
 * Automatically aligns the size to 8 bytes for 64-bit compatibility.
 */
void* kmalloc(size_t size);

/**
 * Marks a previously allocated block as free so it can be reused.
 */
void kfree(void* ptr);

/**
 * Returns the total number of bytes currently marked as allocated.
 */
size_t get_heap_usage();

#endif
