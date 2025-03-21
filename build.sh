#!/bin/bash

# MinOS Build Script
# This script compiles the MinOS operating system and creates a bootable ISO image

set -e  # Exit on error

# Configuration
BUILD_DIR="/home/ubuntu/MinOS-Implementation/build"
SRC_DIR="/home/ubuntu/MinOS-Implementation/src"
ISO_DIR="$BUILD_DIR/iso"
ISO_BOOT_DIR="$ISO_DIR/boot"
ISO_GRUB_DIR="$ISO_BOOT_DIR/grub"
OUTPUT_ISO="$BUILD_DIR/minos.iso"

# Create build directories
mkdir -p $BUILD_DIR
mkdir -p $ISO_DIR
mkdir -p $ISO_BOOT_DIR
mkdir -p $ISO_GRUB_DIR

echo "Building MinOS..."

# Compile kernel
echo "Compiling kernel..."
cd $SRC_DIR/kernel
make clean
make
cp kernel.bin $ISO_BOOT_DIR/

# Compile file system
echo "Compiling file system..."
cd $SRC_DIR/fs
make clean
make
cp fs.o $BUILD_DIR/

# Compile boot system
echo "Compiling boot system..."
cd $SRC_DIR/boot
make clean
make
cp boot.o $BUILD_DIR/

# Compile networking stack
echo "Compiling networking stack..."
cd $SRC_DIR/net
make clean
make
cp net.o $BUILD_DIR/

# Compile shell
echo "Compiling shell..."
cd $SRC_DIR/shell
make clean
make
cp shell.o $BUILD_DIR/

# Compile security and user management
echo "Compiling security and user management..."
cd $SRC_DIR/security
make clean
make
cp security.o $BUILD_DIR/

# Compile package management
echo "Compiling package management..."
cd $SRC_DIR/pkg
make clean
make
cp pkg.o $BUILD_DIR/

# Create GRUB configuration
echo "Creating GRUB configuration..."
cat > $ISO_GRUB_DIR/grub.cfg << EOF
set timeout=5
set default=0

menuentry "MinOS" {
    multiboot /boot/kernel.bin
    boot
}

menuentry "MinOS (Safe Mode)" {
    multiboot /boot/kernel.bin safe_mode=1
    boot
}
EOF

# Create ISO image
echo "Creating bootable ISO image..."
grub-mkrescue -o $OUTPUT_ISO $ISO_DIR

echo "Build complete!"
echo "MinOS ISO image created at: $OUTPUT_ISO"
