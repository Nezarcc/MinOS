#include "memory.h"
#include "kernel.h"
#include <stdint.h>
#include <stddef.h>

/* Memory management globals */
static uint32_t *frames;          // Bitmap of frames
static uint32_t nframes;          // Number of frames
static page_directory_t *kernel_directory = 0;
static page_directory_t *current_directory = 0;

/* Memory allocation tracking */
static uint32_t placement_address = 0x100000; // Start at 1MB
static uint8_t kmalloc_initialized = 0;

/* Frame management macros */
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

/* Set a bit in the frames bitmap */
static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

/* Clear a bit in the frames bitmap */
static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

/* Test if a bit is set */
static uint32_t test_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}

/* Find the first free frame */
static uint32_t first_frame() {
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        if (frames[i] != 0xFFFFFFFF) { // if not all frames are used
            for (j = 0; j < 32; j++) {
                uint32_t toTest = 0x1 << j;
                if (!(frames[i] & toTest)) {
                    return i*32 + j;
                }
            }
        }
    }
    return 0; // No free frames!
}

/* Allocate a frame */
void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    if (page->frame != 0) {
        return; // Frame was already allocated
    }
    
    uint32_t idx = first_frame();
    if (idx == 0) {
        // PANIC! No free frames!
        terminal_writestring("PANIC: No free frames!\n");
        for(;;);
    }
    
    set_frame(idx * PAGE_SIZE);
    page->present = 1;
    page->rw = (is_writeable) ? 1 : 0;
    page->user = (is_kernel) ? 0 : 1;
    page->frame = idx;
}

/* Free a frame */
void free_frame(page_t *page) {
    uint32_t frame;
    if (!(frame = page->frame)) {
        return; // The page didn't have an allocated frame
    }
    
    clear_frame(frame * PAGE_SIZE);
    page->frame = 0;
}

/* Get a page from the page directory */
page_t *get_page(uint32_t address, int make, page_directory_t *dir) {
    // Turn the address into an index
    address /= PAGE_SIZE;
    // Find the page table containing this address
    uint32_t table_idx = address / 1024;
    
    // If this page table is already assigned
    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[address % 1024];
    } else if (make) {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t*)kmalloc_aligned_physical(sizeof(page_table_t), &tmp);
        // Clear the newly allocated page table
        for (int i = 0; i < 1024; i++) {
            dir->tables[table_idx]->pages[i].present = 0;
            dir->tables[table_idx]->pages[i].rw = 0;
            dir->tables[table_idx]->pages[i].user = 0;
            dir->tables[table_idx]->pages[i].frame = 0;
        }
        dir->tables_physical[table_idx] = tmp | 0x7; // Present, RW, User
        return &dir->tables[table_idx]->pages[address % 1024];
    } else {
        return 0;
    }
}

/* Initialize paging */
void paging_init() {
    // Size of physical memory
    uint32_t mem_end_page = 0x1000000; // Assume 16MB of physical RAM
    
    nframes = mem_end_page / PAGE_SIZE;
    frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(nframes));
    
    // Clear the frame bitmap
    for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        frames[i] = 0;
    }
    
    // Create a page directory
    uint32_t phys;
    kernel_directory = (page_directory_t*)kmalloc_aligned_physical(sizeof(page_directory_t), &phys);
    
    // Clear the page directory
    for (int i = 0; i < 1024; i++) {
        kernel_directory->tables[i] = 0;
        kernel_directory->tables_physical[i] = 0;
    }
    
    kernel_directory->physical_addr = phys;
    
    // Identity map the first 4MB
    for (uint32_t i = 0; i < 0x400000; i += PAGE_SIZE) {
        page_t *page = get_page(i, 1, kernel_directory);
        alloc_frame(page, 0, 0);
    }
    
    // Map some pages in the kernel heap area
    for (uint32_t i = KERNEL_VIRTUAL_BASE; i < KERNEL_VIRTUAL_BASE + 0x400000; i += PAGE_SIZE) {
        page_t *page = get_page(i, 1, kernel_directory);
        alloc_frame(page, 0, 0);
    }
    
    // Register page fault handler (to be implemented)
    
    // Enable paging
    current_directory = kernel_directory;
    
    // Enable paging by setting CR0 bit 31
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0":: "r"(cr0));
    
    terminal_writestring("Paging initialized\n");
}

/* Simple physical memory allocator for early boot */
static void* early_kmalloc(size_t size, int align, uint32_t *phys) {
    if (align == 1 && (placement_address & 0xFFFFF000)) {
        // Align the placement address
        placement_address &= 0xFFFFF000;
        placement_address += PAGE_SIZE;
    }
    
    if (phys) {
        *phys = placement_address;
    }
    
    uint32_t tmp = placement_address;
    placement_address += size;
    return (void*)tmp;
}

/* Initialize memory management */
void memory_init() {
    terminal_writestring("Initializing memory management...\n");
    
    // Initialize early kmalloc
    placement_address = 0x100000; // Start at 1MB
    
    // Initialize paging
    paging_init();
    
    // Initialize heap (to be implemented)
    
    kmalloc_initialized = 1;
    
    terminal_writestring("Memory management initialized\n");
}

/* Kernel memory allocation */
void* kmalloc(size_t size) {
    return kmalloc_physical(size, 0);
}

/* Aligned kernel memory allocation */
void* kmalloc_aligned(size_t size) {
    return kmalloc_aligned_physical(size, 0);
}

/* Physical kernel memory allocation */
void* kmalloc_physical(size_t size, uint32_t *physical) {
    if (!kmalloc_initialized) {
        return early_kmalloc(size, 0, physical);
    }
    
    // Heap allocator to be implemented
    return early_kmalloc(size, 0, physical);
}

/* Aligned physical kernel memory allocation */
void* kmalloc_aligned_physical(size_t size, uint32_t *physical) {
    if (!kmalloc_initialized) {
        return early_kmalloc(size, 1, physical);
    }
    
    // Heap allocator to be implemented
    return early_kmalloc(size, 1, physical);
}

/* Kernel memory free */
void kfree(void* ptr) {
    // To be implemented with heap allocator
}
