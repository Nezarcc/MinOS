#include "syscall.h"
#include "interrupt.h"
#include "kernel.h"
#include "process.h"
#include <stdint.h>

/* System call handler function pointers */
static void* syscall_handlers[256] = {0};

/* System call handler */
void syscall_handler() {
    // Get the system call number from EAX
    uint32_t syscall_num;
    asm volatile("mov %%eax, %0" : "=r" (syscall_num));
    
    // Get arguments from other registers
    uint32_t arg1, arg2, arg3, arg4, arg5;
    asm volatile("mov %%ebx, %0" : "=r" (arg1));
    asm volatile("mov %%ecx, %0" : "=r" (arg2));
    asm volatile("mov %%edx, %0" : "=r" (arg3));
    asm volatile("mov %%esi, %0" : "=r" (arg4));
    asm volatile("mov %%edi, %0" : "=r" (arg5));
    
    // Check if the system call is valid
    if (syscall_num >= 256 || syscall_handlers[syscall_num] == 0) {
        // Invalid system call
        terminal_writestring("Invalid system call: ");
        // Print syscall number
        terminal_writestring("\n");
        return;
    }
    
    // Call the system call handler
    typedef int (*syscall_fn_t)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    syscall_fn_t handler = (syscall_fn_t)syscall_handlers[syscall_num];
    int result = handler(arg1, arg2, arg3, arg4, arg5);
    
    // Set the return value in EAX
    asm volatile("mov %0, %%eax" : : "r" (result));
}

/* Register a system call handler */
void register_syscall(uint32_t num, void* handler) {
    if (num < 256) {
        syscall_handlers[num] = handler;
    }
}

/* System call implementations */

/* Exit system call */
static int sys_exit(uint32_t status, uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4) {
    // Terminate the current process
    process_terminate();
    return 0; // Never reached
}

/* Get process ID system call */
static int sys_getpid(uint32_t unused1, uint32_t unused2, uint32_t unused3, uint32_t unused4, uint32_t unused5) {
    process_t* current = process_current();
    if (current) {
        return current->pid;
    }
    return 0;
}

/* Write system call (simplified) */
static int sys_write(uint32_t fd, uint32_t buffer, uint32_t size, uint32_t unused1, uint32_t unused2) {
    // Only support stdout (fd 1) for now
    if (fd == 1) {
        const char* buf = (const char*)buffer;
        for (uint32_t i = 0; i < size; i++) {
            terminal_putchar(buf[i]);
        }
        return size;
    }
    return -1;
}

/* Initialize system call interface */
void syscall_init() {
    terminal_writestring("Initializing system call interface...\n");
    
    // Register system call handlers
    register_syscall(SYS_EXIT, sys_exit);
    register_syscall(SYS_GETPID, sys_getpid);
    register_syscall(SYS_WRITE, sys_write);
    
    // Register interrupt handler for system calls (using int 0x80)
    register_interrupt_handler(0x80, syscall_handler);
    
    terminal_writestring("System call interface initialized\n");
}
