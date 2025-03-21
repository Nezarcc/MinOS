# MinOS File System - Detailed Design

## 1. Introduction

The MinOS file system is designed to be minimalistic, secure, and efficient, providing essential file operations while maintaining a small footprint. This document expands on the file system architecture outlined in the initial design document, providing detailed specifications for implementation.

## 2. File System Architecture

### 2.1 Design Rationale

MinOS implements a lightweight file system that prioritizes simplicity, security, and reliability. The design is influenced by modern file systems but strips away unnecessary complexity to maintain the minimalist philosophy of MinOS.

### 2.2 File System Type

MinOS uses a custom file system called MinFS with the following characteristics:

- **Hierarchical Structure**: Traditional directory tree organization
- **Journaling**: Lightweight journaling for crash recovery
- **Extent-based Allocation**: Efficient storage allocation using extents
- **Copy-on-Write**: Optional CoW for enhanced reliability
- **Encryption Support**: Built-in encryption capabilities
- **Minimal Metadata**: Reduced overhead for file attributes

### 2.3 On-Disk Structure

The MinFS on-disk structure consists of:

1. **Superblock**: Contains file system metadata and configuration
2. **Inode Table**: Stores file and directory metadata
3. **Data Blocks**: Contains actual file and directory data
4. **Extent Maps**: Tracks file data allocation
5. **Journal**: Records pending transactions for crash recovery
6. **Allocation Bitmap**: Tracks free and used blocks

## 3. File and Directory Structure

### 3.1 Inodes

MinFS inodes contain minimal but essential metadata:

- **Inode Number**: Unique identifier for the file/directory
- **File Type**: Regular file, directory, symbolic link, etc.
- **Permissions**: Owner, group, and access permissions
- **Size**: File size in bytes
- **Timestamps**: Creation, modification, and access times (reduced precision)
- **Reference Count**: Number of hard links
- **Data Pointers**: Direct, indirect, and extent pointers
- **Extended Attributes**: Optional additional metadata

### 3.2 Directory Structure

Directories in MinFS are implemented as specialized files:

- **Entry Format**: Fixed-size entries with name and inode number
- **Hashing**: Optional hash-based lookup for large directories
- **Hierarchical Organization**: Standard parent-child relationships
- **Special Entries**: "." (current directory) and ".." (parent directory)

### 3.3 File Types

MinFS supports the following file types:

- **Regular Files**: Standard data files
- **Directories**: Container for other files and directories
- **Symbolic Links**: References to other files or directories
- **Special Files**: Device nodes, pipes, and sockets (minimal implementation)

## 4. Storage Allocation

### 4.1 Block Allocation

MinFS uses an efficient block allocation strategy:

- **Block Size**: Configurable (default: 4KB)
- **Allocation Unit**: Individual blocks or extents
- **Allocation Algorithm**: First-fit or best-fit with bitmap tracking
- **Fragmentation Handling**: Basic defragmentation capabilities

### 4.2 Extent-Based Storage

To reduce metadata overhead, MinFS uses extent-based allocation:

- **Extent Definition**: Contiguous range of blocks (start block + length)
- **Extent Mapping**: Efficient mapping of file logical blocks to physical blocks
- **Extent Merging**: Automatic merging of adjacent extents
- **Extent Splitting**: Splitting extents when necessary for file modifications

### 4.3 Space Management

- **Free Space Tracking**: Bitmap-based free space management
- **Reserved Space**: Small percentage reserved for system operations
- **Quota Support**: Basic user and group quota enforcement
- **Space Monitoring**: Tools for space usage analysis

## 5. File Operations

### 5.1 Basic Operations

MinFS supports standard file operations with minimal overhead:

- **Create**: Create new files and directories
- **Open/Close**: File handle management
- **Read/Write**: Data access with buffering
- **Seek**: Position management within files
- **Truncate**: Resize files
- **Unlink**: Remove files and directories
- **Rename**: Move and rename files and directories
- **Stat**: Retrieve file metadata

### 5.2 Extended Operations

Additional operations with minimal implementation:

- **Symbolic Links**: Create and follow symbolic links
- **Hard Links**: Create additional directory entries for files
- **Extended Attributes**: Store and retrieve additional metadata
- **Access Control Lists**: Optional fine-grained permissions

### 5.3 Performance Optimizations

- **Read-Ahead**: Simple read-ahead for sequential access
- **Write Buffering**: Delayed writes for improved performance
- **Metadata Caching**: Cache frequently accessed metadata
- **Directory Entry Caching**: Cache directory lookups

## 6. Journaling and Recovery

### 6.1 Journal Structure

MinFS implements a lightweight journaling system:

- **Journal Location**: Dedicated area on disk
- **Transaction Format**: Simple record of operations and data
- **Journal Size**: Configurable based on system requirements
- **Checkpointing**: Periodic flushing of completed transactions

### 6.2 Journaling Modes

MinFS supports multiple journaling modes:

- **Metadata-only**: Journal only metadata changes (default)
- **Full**: Journal both metadata and data changes
- **Writeback**: Metadata journaling with delayed data writes
- **None**: Optional mode without journaling for special cases

### 6.3 Recovery Process

After a crash or unclean shutdown:

- **Journal Scan**: Identify incomplete transactions
- **Replay**: Apply or roll back incomplete transactions
- **Consistency Check**: Verify file system integrity
- **Orphan Processing**: Handle files without directory entries

## 7. Security Features

### 7.1 Permission System

MinFS implements a robust permission system:

- **UNIX-style Permissions**: User, group, and other permissions
- **Permission Bits**: Read, write, execute, and special bits
- **Default Permissions**: Configurable default permission masks
- **Permission Inheritance**: Rules for new file/directory permissions

### 7.2 Access Control Lists (ACLs)

Optional extended permission system:

- **ACL Entries**: User and group access rules
- **Permission Granularity**: Fine-grained access control
- **ACL Inheritance**: Rules for propagating ACLs to new files
- **ACL Evaluation**: Algorithm for determining effective permissions

### 7.3 Encryption

MinFS provides built-in encryption capabilities:

- **Encryption Scope**: Per-file, per-directory, or per-volume
- **Encryption Algorithms**: AES-256 in XTS mode
- **Key Management**: Integration with system key management
- **Performance**: Optimized implementation with minimal overhead

### 7.4 Secure Deletion

Support for secure file deletion:

- **Block Overwriting**: Multiple overwrite patterns
- **Metadata Clearing**: Complete removal of file metadata
- **Verification**: Optional verification of secure deletion

## 8. Special Features

### 8.1 Immutable Files

Support for files that cannot be modified:

- **Immutable Flag**: Prevent modifications to critical files
- **Append-only Flag**: Allow only appending to files
- **Temporary Immutability**: Time-limited immutability

### 8.2 Sparse Files

Efficient storage for files with large empty regions:

- **Hole Tracking**: Efficient representation of zero-filled regions
- **Sparse Allocation**: Allocate blocks only for non-zero data
- **Hole Punching**: Create new holes in existing files

### 8.3 Extended Attributes

Storage for additional metadata:

- **Attribute Namespaces**: System, user, security, etc.
- **Attribute Storage**: Inline for small attributes, external for large
- **Attribute Operations**: Get, set, list, and remove

## 9. File System Layout

### 9.1 Standard Directory Hierarchy

MinOS implements a simplified directory hierarchy:

- **/bin**: Essential command binaries
- **/etc**: System configuration files
- **/home**: User home directories
- **/lib**: Shared libraries
- **/sbin**: System binaries
- **/tmp**: Temporary files
- **/usr**: User utilities and applications
- **/var**: Variable data (logs, spool, etc.)

### 9.2 Special File Systems

Support for special-purpose file systems:

- **procfs**: Process and system information
- **devfs**: Device files
- **tmpfs**: In-memory temporary file system
- **sysfs**: System and kernel information

## 10. Implementation Considerations

### 10.1 Programming Language

- **Core Implementation**: C language with strict coding standards
- **Utilities**: Shell scripts for management tools
- **Testing Tools**: C and shell scripts for testing

### 10.2 User Space vs. Kernel Space

- **Primary Implementation**: User space file system service
- **Kernel Interface**: Minimal VFS (Virtual File System) layer in kernel
- **Communication**: Efficient IPC between kernel and file system service

### 10.3 Testing Framework

- **Unit Testing**: Test individual file system components
- **Integration Testing**: Test file system operations and recovery
- **Stress Testing**: Test under heavy load and with limited resources
- **Corruption Testing**: Test recovery from simulated disk errors

## 11. Performance Characteristics

### 11.1 Expected Performance

- **Small File Performance**: Optimized for CLI environment with many small files
- **Sequential Access**: Efficient for log files and configuration files
- **Random Access**: Acceptable performance for database files
- **Metadata Operations**: Fast directory lookups and file creation

### 11.2 Benchmarking

- **Benchmark Suite**: Custom benchmarks for MinFS operations
- **Comparison Metrics**: Size, speed, and reliability compared to other file systems
- **Resource Usage**: Memory, CPU, and I/O utilization measurements

## 12. Future Extensions

### 12.1 Short-term Extensions

- **Enhanced Caching**: More sophisticated caching strategies
- **Improved Defragmentation**: Online defragmentation tools
- **Snapshot Support**: Basic file system snapshot capabilities

### 12.2 Long-term Extensions

- **Distributed Operation**: Optional network file system capabilities
- **Deduplication**: Storage efficiency through data deduplication
- **Advanced Compression**: Transparent file compression

## 13. Limitations and Constraints

- **Maximum File Size**: Limited to 2^63 bytes (practical limit much lower)
- **Maximum Volume Size**: Limited to 2^64 blocks
- **Maximum Filename Length**: 255 bytes
- **Maximum Path Length**: 4096 bytes
- **No Advanced Features**: No built-in RAID, volume management, or snapshots in base implementation
