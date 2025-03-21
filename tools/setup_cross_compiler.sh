#!/bin/bash
# Script to set up a cross-compiler for MinOS development

set -e

# Configuration
export TARGET=i686-elf
export PREFIX="/home/ubuntu/MinOS-Implementation/tools/cross"
export PATH="$PREFIX/bin:$PATH"

# Create directories
mkdir -p $PREFIX
mkdir -p /home/ubuntu/MinOS-Implementation/tools/src

# Download and extract binutils
cd /home/ubuntu/MinOS-Implementation/tools/src
if [ ! -f "binutils-2.37.tar.gz" ]; then
    wget https://ftp.gnu.org/gnu/binutils/binutils-2.37.tar.gz
    tar -xzf binutils-2.37.tar.gz
fi

# Build binutils
mkdir -p build-binutils
cd build-binutils
../binutils-2.37/configure --target=$TARGET --prefix=$PREFIX --with-sysroot --disable-nls --disable-werror
make -j$(nproc)
make install

# Download and extract GCC
cd /home/ubuntu/MinOS-Implementation/tools/src
if [ ! -f "gcc-11.2.0.tar.gz" ]; then
    wget https://ftp.gnu.org/gnu/gcc/gcc-11.2.0/gcc-11.2.0.tar.gz
    tar -xzf gcc-11.2.0.tar.gz
fi

# Build GCC
mkdir -p build-gcc
cd build-gcc
../gcc-11.2.0/configure --target=$TARGET --prefix=$PREFIX --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
make install-gcc
make install-target-libgcc

echo "Cross-compiler setup complete. Add the following to your .bashrc:"
echo "export PATH=\"$PREFIX/bin:\$PATH\""
