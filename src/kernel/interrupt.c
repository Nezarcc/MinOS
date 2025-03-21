#include "interrupt.h"
#include "kernel.h"
#include <stdint.h>

/* IDT entries and pointer */
static idt_entry_t idt_entries[256];
static idt_ptr_t idt_ptr;

/* Interrupt handlers array */
static isr_t interrupt_handlers[256];

/* Set an entry in the IDT */
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = base & 0xFFFF;
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags = flags;
}

/* Initialize the IDT */
static void idt_init() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base = (uint32_t)&idt_entries;
    
    // Clear the IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
        interrupt_handlers[i] = 0;
    }
    
    // Set up ISR handlers
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    
    // Remap the PIC
    // Initialize master PIC
    outb(0x20, 0x11);  // Start initialization sequence (ICW1)
    outb(0xA0, 0x11);  // Start initialization sequence (ICW1)
    
    outb(0x21, 0x20);  // ICW2: Master PIC vector offset (0x20 = 32)
    outb(0xA1, 0x28);  // ICW2: Slave PIC vector offset (0x28 = 40)
    
    outb(0x21, 0x04);  // ICW3: Tell Master PIC that there is a slave PIC at IRQ2
    outb(0xA1, 0x02);  // ICW3: Tell Slave PIC its cascade identity
    
    outb(0x21, 0x01);  // ICW4: 8086/88 mode
    outb(0xA1, 0x01);  // ICW4: 8086/88 mode
    
    outb(0x21, 0x0);   // Enable all IRQs on master PIC
    outb(0xA1, 0x0);   // Enable all IRQs on slave PIC
    
    // Set up IRQ handlers
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
    
    // Load the IDT
    asm volatile("lidt %0" : : "m" (idt_ptr));
}

/* Initialize interrupts */
void interrupts_init() {
    terminal_writestring("Initializing interrupts...\n");
    
    // Initialize the IDT
    idt_init();
    
    terminal_writestring("Interrupts initialized\n");
}

/* Enable interrupts */
void interrupts_enable() {
    asm volatile("sti");
}

/* Disable interrupts */
void interrupts_disable() {
    asm volatile("cli");
}

/* Register an interrupt handler */
void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

/* ISR handler */
void isr_handler() {
    // To be implemented
    terminal_writestring("Received interrupt: ");
    // Print interrupt number
    terminal_writestring("\n");
}

/* IRQ handler */
void irq_handler(uint8_t irq_num) {
    // Send EOI (End of Interrupt) to PICs
    if (irq_num >= 40) {
        // Send reset signal to slave PIC
        outb(0xA0, 0x20);
    }
    // Send reset signal to master PIC
    outb(0x20, 0x20);
    
    // Call handler if registered
    if (interrupt_handlers[irq_num] != 0) {
        isr_t handler = interrupt_handlers[irq_num];
        handler();
    }
}

/* Port I/O functions */
void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
