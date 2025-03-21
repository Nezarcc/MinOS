#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

/* Process states */
typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED
} process_state_t;

/* Process context structure */
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cr3; // Page directory
} process_context_t;

/* Process control block */
typedef struct process {
    uint32_t pid;                  // Process ID
    char name[32];                 // Process name
    process_state_t state;         // Current state
    uint32_t priority;             // Scheduling priority
    process_context_t context;     // CPU context
    uint32_t stack;                // Kernel stack location
    uint32_t stack_size;           // Stack size
    struct process *next;          // Next process in queue
} process_t;

/* Initialize process management */
void process_init(void);

/* Create a new process */
process_t* process_create(const char* name, uint32_t entry_point, uint32_t priority);

/* Schedule the next process to run */
void process_schedule(void);

/* Block the current process */
void process_block(void);

/* Wake up a blocked process */
void process_wake(process_t* process);

/* Terminate the current process */
void process_terminate(void);

/* Get the current running process */
process_t* process_current(void);

/* Switch to a different process */
void process_switch(process_t* process);

#endif /* PROCESS_H */
