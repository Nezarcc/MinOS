#include "process.h"
#include "memory.h"
#include "kernel.h"
#include <stdint.h>
#include <string.h>

/* Process management globals */
static process_t *current_process = NULL;
static process_t *process_list = NULL;
static uint32_t next_pid = 1;

/* Initialize process management */
void process_init() {
    terminal_writestring("Initializing process management...\n");
    
    // Create initial kernel process
    current_process = (process_t*)kmalloc(sizeof(process_t));
    memset(current_process, 0, sizeof(process_t));
    
    current_process->pid = 0;
    strcpy(current_process->name, "kernel");
    current_process->state = PROCESS_STATE_RUNNING;
    current_process->priority = 0;
    current_process->next = NULL;
    
    // Set up kernel stack
    current_process->stack = (uint32_t)kmalloc(4096) + 4096; // Stack grows downward
    current_process->stack_size = 4096;
    
    // Add to process list
    process_list = current_process;
    
    terminal_writestring("Process management initialized\n");
}

/* Create a new process */
process_t* process_create(const char* name, uint32_t entry_point, uint32_t priority) {
    // Allocate process control block
    process_t *process = (process_t*)kmalloc(sizeof(process_t));
    memset(process, 0, sizeof(process_t));
    
    // Set up process information
    process->pid = next_pid++;
    strncpy(process->name, name, 31);
    process->name[31] = '\0';
    process->state = PROCESS_STATE_READY;
    process->priority = priority;
    
    // Allocate stack
    process->stack_size = 4096;
    process->stack = (uint32_t)kmalloc(process->stack_size) + process->stack_size;
    
    // Set up initial context
    process->context.eip = entry_point;
    process->context.eflags = 0x202; // Interrupt enabled
    process->context.esp = process->stack;
    
    // Create page directory (to be implemented)
    
    // Add to process list
    process->next = process_list;
    process_list = process;
    
    return process;
}

/* Schedule the next process to run */
void process_schedule() {
    if (!current_process) {
        return; // No processes to schedule
    }
    
    // Simple round-robin scheduling
    process_t *next = current_process->next;
    if (!next) {
        next = process_list; // Wrap around to start of list
    }
    
    // Skip processes that are not ready
    while (next != current_process && next->state != PROCESS_STATE_READY) {
        next = next->next;
        if (!next) {
            next = process_list; // Wrap around to start of list
        }
    }
    
    // If we found a different process that's ready, switch to it
    if (next != current_process && next->state == PROCESS_STATE_READY) {
        process_switch(next);
    }
}

/* Block the current process */
void process_block() {
    if (!current_process) {
        return;
    }
    
    current_process->state = PROCESS_STATE_BLOCKED;
    process_schedule(); // Find another process to run
}

/* Wake up a blocked process */
void process_wake(process_t* process) {
    if (!process) {
        return;
    }
    
    if (process->state == PROCESS_STATE_BLOCKED) {
        process->state = PROCESS_STATE_READY;
    }
}

/* Terminate the current process */
void process_terminate() {
    if (!current_process) {
        return;
    }
    
    // Mark as terminated
    current_process->state = PROCESS_STATE_TERMINATED;
    
    // Free resources (to be implemented)
    
    // Schedule another process
    process_schedule();
    
    // If we get here, there are no other processes to run
    terminal_writestring("All processes terminated. Halting system.\n");
    for(;;);
}

/* Get the current running process */
process_t* process_current() {
    return current_process;
}

/* Switch to a different process */
void process_switch(process_t* process) {
    if (!process || process == current_process) {
        return;
    }
    
    // Save current process state
    if (current_process->state == PROCESS_STATE_RUNNING) {
        current_process->state = PROCESS_STATE_READY;
    }
    
    // Update current process
    process_t *old_process = current_process;
    current_process = process;
    current_process->state = PROCESS_STATE_RUNNING;
    
    // Switch page directory if needed
    if (current_process->context.cr3 != old_process->context.cr3) {
        // Switch page directory (to be implemented)
    }
    
    // Perform context switch (to be implemented in assembly)
    // context_switch(&old_process->context, &current_process->context);
}
