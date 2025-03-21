# MinOS Kernel Makefile

# Cross-compiler settings
CC = $(TARGET)-gcc
AS = $(TARGET)-as
LD = $(TARGET)-ld

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
ASFLAGS = 
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib

# Source files
C_SOURCES = $(wildcard *.c)
ASM_SOURCES = $(wildcard *.s)

# Object files
OBJECTS = $(ASM_SOURCES:.s=.o) $(C_SOURCES:.c=.o)

# Output file
KERNEL = kernel.bin

# Default target
all: $(KERNEL)

# Compile C files
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Compile assembly files
%.o: %.s
	$(AS) $< -o $@ $(ASFLAGS)

# Link the kernel
$(KERNEL): $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) -lgcc

# Clean build artifacts
clean:
	rm -f $(KERNEL) $(OBJECTS) *.o *~

.PHONY: all clean
