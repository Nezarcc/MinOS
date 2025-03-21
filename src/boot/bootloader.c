#include "bootloader.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <string.h>

/* Multiboot information structure pointer */
static multiboot_info_t* multiboot_info = NULL;

/* Initialize the boot system */
void boot_init(multiboot_info_t* mbi) {
    terminal_writestring("Initializing boot system...\n");
    
    // Save multiboot information
    multiboot_info = mbi;
    
    // Display boot information
    if (multiboot_info->flags & (1 << 0)) {
        terminal_writestring("Memory: Lower=");
        // Print lower memory
        terminal_writestring("KB, Upper=");
        // Print upper memory
        terminal_writestring("KB\n");
    }
    
    if (multiboot_info->flags & (1 << 1)) {
        terminal_writestring("Boot device: ");
        // Print boot device
        terminal_writestring("\n");
    }
    
    if (multiboot_info->flags & (1 << 2)) {
        terminal_writestring("Command line: ");
        char* cmdline = (char*)multiboot_info->cmdline;
        terminal_writestring(cmdline);
        terminal_writestring("\n");
    }
    
    if (multiboot_info->flags & (1 << 3)) {
        terminal_writestring("Modules: count=");
        // Print module count
        terminal_writestring("\n");
    }
    
    if (multiboot_info->flags & (1 << 6)) {
        terminal_writestring("Memory map: ");
        // Print memory map entries
        terminal_writestring("\n");
    }
    
    if (multiboot_info->flags & (1 << 9)) {
        terminal_writestring("Boot loader: ");
        char* bootloader = (char*)multiboot_info->boot_loader_name;
        terminal_writestring(bootloader);
        terminal_writestring("\n");
    }
    
    terminal_writestring("Boot system initialized\n");
}

/* Get available memory information */
void boot_get_memory_info(uint32_t* mem_lower, uint32_t* mem_upper) {
    if (!multiboot_info || !(multiboot_info->flags & (1 << 0))) {
        *mem_lower = 0;
        *mem_upper = 0;
        return;
    }
    
    *mem_lower = multiboot_info->mem_lower;
    *mem_upper = multiboot_info->mem_upper;
}

/* Get memory map */
mmap_entry_t* boot_get_memory_map(uint32_t* count) {
    if (!multiboot_info || !(multiboot_info->flags & (1 << 6))) {
        *count = 0;
        return NULL;
    }
    
    mmap_entry_t* entries = (mmap_entry_t*)multiboot_info->mmap_addr;
    uint32_t length = multiboot_info->mmap_length;
    
    // Count entries
    *count = 0;
    mmap_entry_t* entry = entries;
    while ((uint32_t)entry < (uint32_t)entries + length) {
        (*count)++;
        entry = (mmap_entry_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }
    
    return entries;
}

/* Get command line */
char* boot_get_cmdline(void) {
    if (!multiboot_info || !(multiboot_info->flags & (1 << 2))) {
        return NULL;
    }
    
    return (char*)multiboot_info->cmdline;
}

/* Get boot device */
uint32_t boot_get_boot_device(void) {
    if (!multiboot_info || !(multiboot_info->flags & (1 << 1))) {
        return 0;
    }
    
    return multiboot_info->boot_device;
}

/* Get boot loader name */
char* boot_get_bootloader_name(void) {
    if (!multiboot_info || !(multiboot_info->flags & (1 << 9))) {
        return NULL;
    }
    
    return (char*)multiboot_info->boot_loader_name;
}
