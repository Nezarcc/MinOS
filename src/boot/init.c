#include "init.h"
#include "bootloader.h"
#include "initrd.h"
#include "../fs/vfs.h"
#include "../fs/minfs.h"
#include "../fs/file.h"
#include "../kernel/kernel.h"
#include "../kernel/memory.h"
#include "../kernel/interrupt.h"
#include "../kernel/process.h"
#include "../kernel/syscall.h"
#include "../kernel/timer.h"
#include <stdint.h>
#include <string.h>

/* Initialize the system */
void init_system() {
    terminal_writestring("Initializing MinOS system...\n");
    
    // Initialize memory management
    memory_init();
    
    // Initialize interrupts
    interrupts_init();
    
    // Initialize system timer (100Hz)
    timer_init(100);
    
    // Initialize process management
    process_init();
    
    // Initialize system call interface
    syscall_init();
    
    // Initialize file systems
    vfs_init();
    minfs_init();
    file_init();
    
    // Enable interrupts
    interrupts_enable();
    
    terminal_writestring("System initialization complete\n");
}

/* Mount the root file system */
int init_mount_root() {
    terminal_writestring("Mounting root file system...\n");
    
    // Try to mount the root file system from the boot device
    uint32_t boot_device = boot_get_boot_device();
    
    // For now, we'll use the initial ramdisk as the root file system
    // In a real implementation, we would detect and mount the actual root device
    
    // Get the location of the initial ramdisk from multiboot info
    // This would typically be passed by the bootloader
    uint32_t initrd_location = 0x200000; // Example location, would be determined dynamically
    
    // Initialize the initial ramdisk
    fs_node_t* initrd_root = initrd_init(initrd_location);
    if (!initrd_root) {
        terminal_writestring("Failed to initialize initial ramdisk\n");
        return -1;
    }
    
    // Mount the initial ramdisk as the root file system
    if (vfs_mount("/", initrd_root) != 0) {
        terminal_writestring("Failed to mount initial ramdisk as root file system\n");
        return -1;
    }
    
    terminal_writestring("Root file system mounted\n");
    return 0;
}

/* Load and execute init process */
int init_exec() {
    terminal_writestring("Starting init process...\n");
    
    // In a real implementation, we would load the init program from the file system
    // and create a process to execute it
    
    // For now, we'll just create a simple init process
    process_t* init = process_create("init", 0, 0);
    if (!init) {
        terminal_writestring("Failed to create init process\n");
        return -1;
    }
    
    terminal_writestring("Init process started\n");
    return 0;
}
