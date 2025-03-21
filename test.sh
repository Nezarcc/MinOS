#!/bin/bash

# MinOS Test Script
# This script runs the MinOS operating system in QEMU for testing

set -e  # Exit on error

# Configuration
BUILD_DIR="/home/ubuntu/MinOS-Implementation/build"
ISO_FILE="$BUILD_DIR/minos.iso"

# Check if ISO exists
if [ ! -f "$ISO_FILE" ]; then
    echo "Error: MinOS ISO not found at $ISO_FILE"
    echo "Please run build.sh first to create the ISO image."
    exit 1
fi

echo "Starting MinOS in QEMU..."
echo "Press Ctrl+Alt+G to release mouse, Ctrl+Alt+X to exit QEMU"

# Run QEMU with the MinOS ISO
qemu-system-x86_64 -cdrom "$ISO_FILE" -boot d -m 512 -serial stdio
