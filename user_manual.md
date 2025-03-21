# MinOS User Manual

## Introduction

MinOS (pronounced "Minus") is an ultra-lightweight, secure, and fast operating system that provides only the essential features needed for an efficient and robust CLI-based environment. MinOS is designed for power users, developers, and embedded systems that require a minimal footprint while maintaining strong security and performance.

## Core Principles

1. **Minimalism** – No unnecessary features, no bloat, and an ultra-light system.
2. **Speed** – Optimized kernel and utilities for maximum efficiency.
3. **Security** – Hardened system architecture with strong user controls.
4. **Reliability** – Fail-safe mechanisms to ensure system stability.
5. **Portability** – Designed to run on low-spec devices, virtual machines, and embedded systems.
6. **Extensibility** – Allows users to add custom functionalities while keeping the base system minimal.

## Building MinOS

To build MinOS from source, follow these steps:

1. Ensure you have the required dependencies installed:
   - GCC and build tools
   - NASM assembler
   - QEMU for testing
   - GRUB bootloader utilities

2. Run the build script:
   ```
   ./build.sh
   ```

3. This will create a bootable ISO image at `build/minos.iso`

## Testing MinOS

To test MinOS in an emulator, run the test script:

```
./test.sh
```

This will start QEMU with the MinOS ISO image. You can interact with the MinOS shell and test its features.

## Key Components

### Kernel

MinOS uses a custom microkernel architecture that provides:
- Efficient process management and scheduling
- Minimal memory footprint
- Multi-user and multi-session support
- Secure execution environments

### File System

The MinFS file system offers:
- Lightweight hierarchical structure
- Strong permission controls
- Encrypted home directories
- Journaling for crash recovery

### Networking

The networking stack includes:
- Minimal TCP/IP implementation
- Secure network interfaces
- Built-in firewall
- No default open ports

### Shell

The MinShell provides:
- Command auto-completion
- History management
- Scripting capabilities
- System monitoring tools

### Security

Security features include:
- Role-based access control
- Modern password hashing (Argon2)
- Process isolation
- Mandatory system updates for security patches

### Package Management

The package manager offers:
- Simple installation of pre-vetted utilities
- Minimal dependencies
- Secure software repository
- Support for statically linked binaries

## Basic Commands

MinOS includes essential commands for system management:

- `help` - Display available commands
- `ls` - List directory contents
- `cd` - Change directory
- `cat` - Display file contents
- `mkdir` - Create directory
- `rm` - Remove file
- `ps` - Show process status
- `kill` - Terminate process
- `ifconfig` - Configure network interfaces
- `ping` - Test network connectivity
- `pkg` - Package management

## System Administration

### User Management

- `useradd` - Add a new user
- `userdel` - Delete a user
- `passwd` - Change password
- `groups` - List user groups

### Package Management

- `pkg install <package>` - Install a package
- `pkg remove <package>` - Remove a package
- `pkg update` - Update package database
- `pkg upgrade` - Upgrade installed packages

### System Maintenance

- `update` - Check for system updates
- `upgrade` - Install system updates
- `shutdown` - Shut down the system
- `reboot` - Restart the system

## Conclusion

MinOS provides a barebones, no-nonsense CLI-based OS that strips away all unnecessary complexities, leaving only a secure, fast, and efficient system that is easy to use and extend when needed. It is designed for power users, developers, and low-resource environments, ensuring maximum control, security, and stability.
