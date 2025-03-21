# MinOS Build System

# Configuration
TARGET = i686-elf
PREFIX = /home/ubuntu/MinOS-Implementation/tools/cross
CROSS = $(PREFIX)/bin/$(TARGET)

# Compiler and flags
CC = $(CROSS)-gcc
AS = $(CROSS)-as
LD = $(CROSS)-ld
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LDFLAGS = -ffreestanding -O2 -nostdlib

# Directories
SRC_DIR = src
BUILD_DIR = build
KERNEL_SRC = $(SRC_DIR)/kernel
BOOT_SRC = $(SRC_DIR)/boot
FS_SRC = $(SRC_DIR)/fs
NET_SRC = $(SRC_DIR)/net
SHELL_SRC = $(SRC_DIR)/shell
SECURITY_SRC = $(SRC_DIR)/security
PKG_SRC = $(SRC_DIR)/pkg
UTILS_SRC = $(SRC_DIR)/utils

# Output files
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
ISO_FILE = $(BUILD_DIR)/minos.iso

# Default target
all: $(ISO_FILE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile kernel
$(KERNEL_BIN): $(BUILD_DIR)
	# This will be expanded as kernel source files are created
	@echo "Building kernel..."

# Create bootable ISO
$(ISO_FILE): $(KERNEL_BIN)
	@echo "Creating bootable ISO..."
	# This will be expanded with grub and ISO creation commands

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)/*

# Run in QEMU
run: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE)

# Debug with QEMU and GDB
debug: $(ISO_FILE)
	qemu-system-i386 -cdrom $(ISO_FILE) -s -S

.PHONY: all clean run debug
