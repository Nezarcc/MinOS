#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

/* Memory management constants */
#define PAGE_SIZE 4096
#define KERNEL_VIRTUAL_BASE 0xC0000000
#define KERNEL_PAGE_NUMBER (KERNEL_VIRTUAL_BASE >> 22)

/* Memory management structures */
typedef struct page {
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw         : 1;   // Read-only if clear, readwrite if set
    uint32_t user       : 1;   // Supervisor level only if clear
    uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
    uint32_t dirty      : 1;   // Has the page been written to since last refresh?
    uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
    uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    /* Array of pointers to page tables */
    page_table_t *tables[1024];
    /* Array of pointers to the page tables above, but gives their physical
     * location for loading into the CR3 register. */
    uint32_t tables_physical[1024];
    /* The physical address of tables_physical. This is needed once kernel page
     * directory has been switched to. */
    uint32_t physical_addr;
} page_directory_t;

/* Memory management functions */
void memory_init(void);
void paging_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* kmalloc_aligned(size_t size);
void* kmalloc_physical(size_t size, uint32_t *physical);
void* kmalloc_aligned_physical(size_t size, uint32_t *physical);

/* Page fault handler */
void page_fault_handler(void);

#endif /* MEMORY_H */
