# MinOS Kernel Architecture - Detailed Design

## 1. Introduction

The MinOS kernel is the core component of the operating system, providing essential services while maintaining a minimal footprint. This document expands on the kernel architecture outlined in the initial design document, providing detailed specifications for implementation.

## 2. Microkernel Architecture

### 2.1 Design Rationale

MinOS employs a microkernel architecture to achieve its goals of minimalism, security, and reliability. The microkernel approach offers several advantages:

- **Reduced TCB (Trusted Computing Base)**: Only essential services run in kernel space, minimizing the attack surface
- **Improved Stability**: Failures in device drivers or system services don't crash the entire system
- **Modularity**: Components can be developed, tested, and updated independently
- **Security**: Strong isolation between components enhances overall system security

### 2.2 Kernel Space Components

The MinOS microkernel includes only the following essential components in kernel space:

1. **Process Management**: Thread scheduling, context switching, and basic process control
2. **Memory Management**: Virtual memory, physical memory allocation, and memory protection
3. **Inter-Process Communication (IPC)**: Message passing primitives and synchronization mechanisms
4. **Hardware Abstraction Layer (HAL)**: Minimal hardware abstraction for architecture-specific functions
5. **System Call Interface**: Secure gateway for user space to kernel space transitions

### 2.3 User Space Components

All other traditional kernel services are implemented in user space, including:

1. **Device Drivers**: Storage, network, input/output device management
2. **File Systems**: File operations, directory management, and storage allocation
3. **Network Stack**: Protocol implementations and network interface management
4. **Security Services**: Authentication, access control, and audit mechanisms
5. **System Utilities**: Basic system management tools

## 3. Process Management

### 3.1 Process Model

MinOS implements a lightweight process model with the following characteristics:

- **Process Structure**: Each process contains one or more threads sharing an address space
- **Process States**: Created, Ready, Running, Blocked, Terminated
- **Process Control Block (PCB)**: Minimal data structure containing:
  - Process ID and parent process ID
  - Memory map and allocation information
  - Open file descriptors
  - Security context and permissions
  - Scheduling information
  - Resource usage statistics

### 3.2 Thread Model

- **Thread Structure**: Lightweight execution context within a process
- **Thread States**: Created, Ready, Running, Blocked, Terminated
- **Thread Control Block (TCB)**: Contains:
  - Thread ID
  - Register context
  - Stack pointer
  - Scheduling information
  - Thread-local storage reference

### 3.3 Scheduling

MinOS employs a priority-based preemptive scheduler optimized for CLI workloads:

- **Scheduling Algorithm**: Multilevel feedback queue with priority aging
- **Time Slices**: Variable time quantum based on priority and process type
- **Priority Levels**: 32 priority levels (0-31, with 0 being highest)
- **Priority Inheritance**: Implemented to prevent priority inversion
- **CPU Affinity**: Support for processor affinity on multi-core systems
- **Idle Thread**: Low-priority thread that runs when no other threads are ready

### 3.4 Process Creation and Termination

- **Process Creation**: Lightweight fork() implementation with copy-on-write
- **Process Loading**: Efficient executable loading with minimal overhead
- **Process Termination**: Clean resource reclamation and parent notification
- **Zombie Prevention**: Automatic reaping of terminated processes

## 4. Memory Management

### 4.1 Virtual Memory System

MinOS implements a minimal but efficient virtual memory system:

- **Address Space**: 64-bit virtual address space (for x86_64 architecture)
- **Page Size**: 4KB standard pages with support for larger pages when beneficial
- **Page Tables**: Multi-level page tables with minimal memory overhead
- **Memory Regions**: Code, data, heap, stack, and shared memory regions

### 4.2 Physical Memory Management

- **Page Frame Allocator**: Buddy system allocator for physical memory
- **Memory Zones**: Division of physical memory into zones based on accessibility
- **Page Replacement**: Simple FIFO or Clock algorithm for page replacement
- **Memory Compaction**: Basic compaction to reduce fragmentation

### 4.3 Memory Protection

- **Protection Domains**: Strict separation between process address spaces
- **Memory Permissions**: Read, write, execute permissions at page granularity
- **Kernel/User Separation**: Strict separation between kernel and user address spaces
- **ASLR (Address Space Layout Randomization)**: Basic implementation for security

### 4.4 Memory Allocation

- **Kernel Memory Allocator**: Slab allocator for kernel objects
- **User Space Allocator Interface**: Minimal system calls for memory management
- **Shared Memory**: Controlled mechanism for inter-process memory sharing
- **Memory Mapping**: Support for file-backed and anonymous mappings

## 5. Inter-Process Communication (IPC)

### 5.1 Message Passing

MinOS uses message passing as the primary IPC mechanism:

- **Message Structure**: Simple message format with header and payload
- **Synchronous Messaging**: Blocking send/receive operations
- **Asynchronous Messaging**: Non-blocking message queues
- **Message Routing**: Direct process-to-process communication
- **Message Security**: Capability-based access control for message endpoints

### 5.2 Synchronization Primitives

- **Mutexes**: Lightweight mutual exclusion locks
- **Semaphores**: Counting semaphores for resource management
- **Condition Variables**: For thread synchronization
- **Read-Write Locks**: For concurrent read access with exclusive write access
- **Futexes**: Fast user-space mutex implementation

### 5.3 Shared Memory

- **Shared Memory Regions**: Controlled sharing of memory between processes
- **Memory Protection**: Permission control for shared regions
- **Synchronization**: Integration with IPC synchronization primitives

## 6. Hardware Abstraction Layer (HAL)

### 6.1 Architecture Support

MinOS initially targets the following architectures:

- **x86_64**: Primary development platform
- **ARM64**: Secondary platform for embedded systems

### 6.2 HAL Components

- **CPU Abstraction**: Processor initialization and management
- **Interrupt Controller**: Interrupt routing and handling
- **Timer Services**: System clock and timer management
- **Memory Management Unit**: Architecture-specific memory management
- **I/O Port Access**: Controlled access to hardware I/O ports

### 6.3 Device Interface

- **Device Abstraction**: Minimal interface for device drivers
- **Interrupt Handling**: Registration and dispatching of hardware interrupts
- **DMA Support**: Basic DMA operations for device drivers
- **Power Management**: Essential power state transitions

## 7. System Call Interface

### 7.1 System Call Mechanism

- **Fast Path**: Optimized system call entry and exit
- **Parameter Validation**: Strict validation of all system call parameters
- **Error Handling**: Consistent error reporting mechanism
- **Privilege Checking**: Security validation before execution

### 7.2 Core System Calls

MinOS provides a minimal set of system calls, including:

- **Process Management**: create, terminate, wait, yield
- **Memory Management**: allocate, free, map, protect
- **File Operations**: open, close, read, write, seek
- **IPC**: send_message, receive_message, create_endpoint
- **Time Services**: get_time, set_alarm, sleep
- **Security**: set_permissions, check_access, get_credentials

## 8. Security Features

### 8.1 Kernel Security

- **Privilege Levels**: Strict enforcement of kernel/user mode separation
- **System Call Validation**: Thorough validation of all system call parameters
- **Resource Limits**: Per-process resource usage limits
- **Stack Protection**: Guard pages and stack canaries
- **Kernel Address Space Protection**: KASLR (Kernel Address Space Layout Randomization)

### 8.2 Process Isolation

- **Address Space Separation**: Complete isolation between process address spaces
- **Controlled Sharing**: Explicit mechanisms for controlled resource sharing
- **Capability-Based Security**: Fine-grained access control for system resources
- **Sandboxing**: Support for process sandboxing and restricted execution environments

## 9. Performance Optimizations

### 9.1 Critical Path Optimization

- **Fast Context Switching**: Optimized thread context switching
- **Efficient IPC**: Low-latency message passing
- **Minimal Locking**: Fine-grained locking and lock-free algorithms where possible
- **Cache-Friendly Design**: Data structures and algorithms optimized for cache efficiency

### 9.2 Memory Optimizations

- **Copy-on-Write**: Reduced memory copying during process creation
- **Page Sharing**: Shared pages for identical memory content
- **Memory Prefetching**: Strategic prefetching for common access patterns
- **Cache Management**: Explicit cache control for performance-critical operations

## 10. Implementation Considerations

### 10.1 Programming Language

- **Core Kernel**: C language with strict coding standards
- **Assembly**: Used only for architecture-specific and performance-critical sections
- **No Dynamic Memory**: Fixed-size allocations in critical paths

### 10.2 Build System

- **Minimal Build Dependencies**: Simple Makefile-based build system
- **Cross-Compilation Support**: Build tools for multiple target architectures
- **Optimization Levels**: Different optimization profiles for debug/release

### 10.3 Testing Framework

- **Unit Testing**: Framework for testing individual kernel components
- **Integration Testing**: Testing of component interactions
- **Stress Testing**: Tools for kernel stability under load
- **Performance Testing**: Benchmarking critical kernel operations

## 11. Future Extensions

### 11.1 Short-term Extensions

- **SMP Support**: Symmetric multiprocessing for multi-core systems
- **Enhanced Power Management**: Advanced power saving features
- **Additional Architecture Support**: RISC-V architecture support

### 11.2 Long-term Extensions

- **Real-time Extensions**: Optional real-time scheduling capabilities
- **Virtualization Support**: Lightweight hypervisor functionality
- **Advanced Security Features**: Mandatory access control frameworks

## 12. Limitations and Constraints

- **No Monolithic Drivers**: All device drivers must run in user space
- **Limited Hardware Support**: Focus on essential and common hardware
- **No Legacy Support**: No support for legacy or deprecated interfaces
- **Minimal Feature Set**: Only essential kernel features implemented
