#!/bin/bash
# Script to create a bootable ISO image for MinOS

set -e

# Configuration
ISO_DIR=/home/ubuntu/MinOS-Implementation/build/iso
KERNEL_BIN=/home/ubuntu/MinOS-Implementation/build/kernel.bin

# Create ISO directory structure
mkdir -p $ISO_DIR/boot/grub

# Copy kernel binary
cp $KERNEL_BIN $ISO_DIR/boot/

# Create GRUB configuration
cat > $ISO_DIR/boot/grub/grub.cfg << EOF
set timeout=0
set default=0

menuentry "MinOS" {
    multiboot /boot/kernel.bin
    boot
}
EOF

# Create the ISO image
grub-mkrescue -o /home/ubuntu/MinOS-Implementation/build/minos.iso $ISO_DIR

echo "MinOS ISO image created at /home/ubuntu/MinOS-Implementation/build/minos.iso"
