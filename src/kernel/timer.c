#include "timer.h"
#include "interrupt.h"
#include "kernel.h"
#include "process.h"
#include <stdint.h>

/* Timer variables */
static uint32_t tick = 0;
static uint32_t timer_frequency = 0;

/* Timer interrupt handler */
static void timer_callback() {
    tick++;
    
    // Every 100 ticks (about 100ms at 1000Hz), perform a context switch
    if (tick % 100 == 0) {
        // Call the scheduler
        process_schedule();
    }
}

/* Initialize the system timer */
void timer_init(uint32_t frequency) {
    terminal_writestring("Initializing system timer...\n");
    
    // Save the timer frequency
    timer_frequency = frequency;
    
    // Register the timer callback
    register_interrupt_handler(IRQ0, timer_callback);
    
    // The PIT (Programmable Interval Timer) operates at 1.193182MHz
    uint32_t divisor = 1193180 / frequency;
    
    // Send the command byte
    outb(0x43, 0x36); // Command: channel 0, square wave generator, LSB/MSB, binary
    
    // Send the divisor
    outb(0x40, divisor & 0xFF);         // Low byte
    outb(0x40, (divisor >> 8) & 0xFF);  // High byte
    
    terminal_writestring("System timer initialized at ");
    // Print frequency
    terminal_writestring(" Hz\n");
}

/* Get the current tick count */
uint32_t timer_get_ticks() {
    return tick;
}

/* Sleep for a specified number of ticks */
void timer_sleep(uint32_t ticks) {
    uint32_t start = tick;
    
    // Wait until enough ticks have passed
    while (tick - start < ticks) {
        // Yield to other processes
        asm volatile("hlt");
    }
}
