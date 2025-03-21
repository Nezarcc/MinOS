#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <stdint.h>

/* Interrupt Descriptor Table (IDT) entry structure */
struct idt_entry_struct {
    uint16_t base_lo;    // Lower 16 bits of handler function address
    uint16_t sel;        // Kernel segment selector
    uint8_t  always0;    // Always zero
    uint8_t  flags;      // Flags
    uint16_t base_hi;    // Upper 16 bits of handler function address
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

/* IDT pointer structure */
struct idt_ptr_struct {
    uint16_t limit;      // Size of IDT - 1
    uint32_t base;       // Address of first IDT entry
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

/* Interrupt handler function type */
typedef void (*isr_t)(void);

/* Interrupt handler registration function */
void register_interrupt_handler(uint8_t n, isr_t handler);

/* Initialize interrupts */
void interrupts_init(void);

/* Enable interrupts */
void interrupts_enable(void);

/* Disable interrupts */
void interrupts_disable(void);

/* ISR handlers */
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

/* IRQ handlers */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/* IRQ numbers */
#define IRQ0  32 // Timer
#define IRQ1  33 // Keyboard
#define IRQ2  34 // Cascade for 8259A Slave controller
#define IRQ3  35 // Serial port 2
#define IRQ4  36 // Serial port 1
#define IRQ5  37 // Parallel port 2 or sound card
#define IRQ6  38 // Floppy disk controller
#define IRQ7  39 // Parallel port 1
#define IRQ8  40 // Real-time clock
#define IRQ9  41 // Redirected IRQ2
#define IRQ10 42 // Reserved
#define IRQ11 43 // Reserved
#define IRQ12 44 // PS/2 mouse
#define IRQ13 45 // FPU, coprocessor or inter-processor
#define IRQ14 46 // Primary ATA hard disk
#define IRQ15 47 // Secondary ATA hard disk

#endif /* INTERRUPT_H */
