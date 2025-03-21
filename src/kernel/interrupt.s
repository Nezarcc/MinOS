; Interrupt Service Routines
; This file contains the interrupt service routine stubs

; Macro to create an ISR with no error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli                     ; Disable interrupts
    push byte 0             ; Push dummy error code
    push byte %1            ; Push interrupt number
    jmp isr_common_stub     ; Jump to common handler
%endmacro

; Macro to create an ISR with error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli                     ; Disable interrupts
    push byte %1            ; Push interrupt number
    jmp isr_common_stub     ; Jump to common handler
%endmacro

; Macro to create an IRQ handler
%macro IRQ 2
global irq%1
irq%1:
    cli                     ; Disable interrupts
    push byte 0             ; Push dummy error code
    push byte %2            ; Push interrupt number
    jmp irq_common_stub     ; Jump to common handler
%endmacro

; Define ISRs for CPU exceptions
ISR_NOERRCODE 0    ; Division by zero
ISR_NOERRCODE 1    ; Debug
ISR_NOERRCODE 2    ; Non-maskable interrupt
ISR_NOERRCODE 3    ; Breakpoint
ISR_NOERRCODE 4    ; Overflow
ISR_NOERRCODE 5    ; Bound range exceeded
ISR_NOERRCODE 6    ; Invalid opcode
ISR_NOERRCODE 7    ; Device not available
ISR_ERRCODE   8    ; Double fault
ISR_NOERRCODE 9    ; Coprocessor segment overrun
ISR_ERRCODE   10   ; Invalid TSS
ISR_ERRCODE   11   ; Segment not present
ISR_ERRCODE   12   ; Stack-segment fault
ISR_ERRCODE   13   ; General protection fault
ISR_ERRCODE   14   ; Page fault
ISR_NOERRCODE 15   ; Reserved
ISR_NOERRCODE 16   ; x87 FPU floating-point error
ISR_ERRCODE   17   ; Alignment check
ISR_NOERRCODE 18   ; Machine check
ISR_NOERRCODE 19   ; SIMD floating-point exception
ISR_NOERRCODE 20   ; Virtualization exception
ISR_NOERRCODE 21   ; Reserved
ISR_NOERRCODE 22   ; Reserved
ISR_NOERRCODE 23   ; Reserved
ISR_NOERRCODE 24   ; Reserved
ISR_NOERRCODE 25   ; Reserved
ISR_NOERRCODE 26   ; Reserved
ISR_NOERRCODE 27   ; Reserved
ISR_NOERRCODE 28   ; Reserved
ISR_NOERRCODE 29   ; Reserved
ISR_NOERRCODE 30   ; Security exception
ISR_NOERRCODE 31   ; Reserved

; Define IRQs
IRQ 0, 32   ; Timer
IRQ 1, 33   ; Keyboard
IRQ 2, 34   ; Cascade for 8259A Slave controller
IRQ 3, 35   ; Serial port 2
IRQ 4, 36   ; Serial port 1
IRQ 5, 37   ; Parallel port 2 or sound card
IRQ 6, 38   ; Floppy disk controller
IRQ 7, 39   ; Parallel port 1
IRQ 8, 40   ; Real-time clock
IRQ 9, 41   ; Redirected IRQ2
IRQ 10, 42  ; Reserved
IRQ 11, 43  ; Reserved
IRQ 12, 44  ; PS/2 mouse
IRQ 13, 45  ; FPU, coprocessor or inter-processor
IRQ 14, 46  ; Primary ATA hard disk
IRQ 15, 47  ; Secondary ATA hard disk

; Common ISR stub
extern isr_handler
isr_common_stub:
    ; Save registers
    pusha
    
    ; Save data segment
    mov ax, ds
    push eax
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler
    call isr_handler
    
    ; Restore data segment
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore registers
    popa
    
    ; Clean up error code and ISR number
    add esp, 8
    
    ; Return from interrupt
    iret

; Common IRQ stub
extern irq_handler
irq_common_stub:
    ; Save registers
    pusha
    
    ; Save data segment
    mov ax, ds
    push eax
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler
    call irq_handler
    
    ; Restore data segment
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore registers
    popa
    
    ; Clean up error code and IRQ number
    add esp, 8
    
    ; Return from interrupt
    iret
