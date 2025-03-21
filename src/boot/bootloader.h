#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>

/* Multiboot header constants */
#define MULTIBOOT_MAGIC        0x1BADB002
#define MULTIBOOT_FLAGS        0x00000003  /* Align modules + provide memory map */
#define MULTIBOOT_CHECKSUM     -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

/* Multiboot information structure */
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
} multiboot_info_t;

/* Memory map entry */
typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) mmap_entry_t;

/* Initialize the boot system */
void boot_init(multiboot_info_t* mbi);

/* Get available memory information */
void boot_get_memory_info(uint32_t* mem_lower, uint32_t* mem_upper);

/* Get memory map */
mmap_entry_t* boot_get_memory_map(uint32_t* count);

/* Get command line */
char* boot_get_cmdline(void);

/* Get boot device */
uint32_t boot_get_boot_device(void);

/* Get boot loader name */
char* boot_get_bootloader_name(void);

#endif /* BOOTLOADER_H */
