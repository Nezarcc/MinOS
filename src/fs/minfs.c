#include "minfs.h"
#include "vfs.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* MinFS magic number */
#define MINFS_MAGIC 0x4D494E46 /* "MINF" */

/* MinFS version */
#define MINFS_VERSION 0x0001

/* Block size (4KB) */
#define MINFS_BLOCK_SIZE 4096

/* Maximum file name length */
#define MINFS_MAX_NAME_LEN 255

/* File types */
#define MINFS_TYPE_FILE      0x01
#define MINFS_TYPE_DIRECTORY 0x02
#define MINFS_TYPE_SYMLINK   0x03

/* MinFS in-memory superblock */
static minfs_superblock_t* minfs_sb = NULL;

/* Device node for the mounted file system */
static fs_node_t* minfs_device = NULL;

/* Read a block from the device */
static int minfs_read_block(uint32_t block, uint8_t* buffer) {
    if (!minfs_device) {
        return -1;
    }
    
    uint32_t offset = block * MINFS_BLOCK_SIZE;
    return vfs_read(minfs_device, offset, MINFS_BLOCK_SIZE, buffer);
}

/* Write a block to the device */
static int minfs_write_block(uint32_t block, uint8_t* buffer) {
    if (!minfs_device) {
        return -1;
    }
    
    uint32_t offset = block * MINFS_BLOCK_SIZE;
    return vfs_write(minfs_device, offset, MINFS_BLOCK_SIZE, buffer);
}

/* Read an inode from the device */
static int minfs_read_inode(uint32_t inode_num, minfs_inode_t* inode) {
    if (!minfs_sb || inode_num >= minfs_sb->inode_count) {
        return -1;
    }
    
    uint32_t inode_block = minfs_sb->inode_table_block + (inode_num * sizeof(minfs_inode_t)) / MINFS_BLOCK_SIZE;
    uint32_t inode_offset = (inode_num * sizeof(minfs_inode_t)) % MINFS_BLOCK_SIZE;
    
    uint8_t block_buffer[MINFS_BLOCK_SIZE];
    if (minfs_read_block(inode_block, block_buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    memcpy(inode, block_buffer + inode_offset, sizeof(minfs_inode_t));
    return 0;
}

/* Write an inode to the device */
static int minfs_write_inode(uint32_t inode_num, minfs_inode_t* inode) {
    if (!minfs_sb || inode_num >= minfs_sb->inode_count) {
        return -1;
    }
    
    uint32_t inode_block = minfs_sb->inode_table_block + (inode_num * sizeof(minfs_inode_t)) / MINFS_BLOCK_SIZE;
    uint32_t inode_offset = (inode_num * sizeof(minfs_inode_t)) % MINFS_BLOCK_SIZE;
    
    uint8_t block_buffer[MINFS_BLOCK_SIZE];
    if (minfs_read_block(inode_block, block_buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    memcpy(block_buffer + inode_offset, inode, sizeof(minfs_inode_t));
    
    if (minfs_write_block(inode_block, block_buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    return 0;
}

/* Allocate a new inode */
static uint32_t minfs_alloc_inode() {
    if (!minfs_sb || minfs_sb->free_inodes == 0) {
        return 0; // No free inodes
    }
    
    uint8_t bitmap[MINFS_BLOCK_SIZE];
    if (minfs_read_block(minfs_sb->inode_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
        return 0;
    }
    
    // Find a free inode in the bitmap
    for (uint32_t i = 0; i < minfs_sb->inode_count; i++) {
        uint32_t byte_index = i / 8;
        uint32_t bit_index = i % 8;
        
        if (!(bitmap[byte_index] & (1 << bit_index))) {
            // Found a free inode, mark it as used
            bitmap[byte_index] |= (1 << bit_index);
            
            // Write the updated bitmap
            if (minfs_write_block(minfs_sb->inode_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
                return 0;
            }
            
            // Update superblock
            minfs_sb->free_inodes--;
            
            // Write the updated superblock
            uint8_t sb_buffer[MINFS_BLOCK_SIZE];
            memcpy(sb_buffer, minfs_sb, sizeof(minfs_superblock_t));
            if (minfs_write_block(0, sb_buffer) != MINFS_BLOCK_SIZE) {
                return 0;
            }
            
            return i;
        }
    }
    
    return 0; // No free inodes found
}

/* Allocate a new block */
static uint32_t minfs_alloc_block() {
    if (!minfs_sb || minfs_sb->free_blocks == 0) {
        return 0; // No free blocks
    }
    
    uint8_t bitmap[MINFS_BLOCK_SIZE];
    if (minfs_read_block(minfs_sb->block_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
        return 0;
    }
    
    // Find a free block in the bitmap
    for (uint32_t i = 0; i < minfs_sb->block_count; i++) {
        uint32_t byte_index = i / 8;
        uint32_t bit_index = i % 8;
        
        if (!(bitmap[byte_index] & (1 << bit_index))) {
            // Found a free block, mark it as used
            bitmap[byte_index] |= (1 << bit_index);
            
            // Write the updated bitmap
            if (minfs_write_block(minfs_sb->block_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
                return 0;
            }
            
            // Update superblock
            minfs_sb->free_blocks--;
            
            // Write the updated superblock
            uint8_t sb_buffer[MINFS_BLOCK_SIZE];
            memcpy(sb_buffer, minfs_sb, sizeof(minfs_superblock_t));
            if (minfs_write_block(0, sb_buffer) != MINFS_BLOCK_SIZE) {
                return 0;
            }
            
            return minfs_sb->data_block_start + i;
        }
    }
    
    return 0; // No free blocks found
}

/* Free an inode */
static int minfs_free_inode(uint32_t inode_num) {
    if (!minfs_sb || inode_num >= minfs_sb->inode_count) {
        return -1;
    }
    
    uint8_t bitmap[MINFS_BLOCK_SIZE];
    if (minfs_read_block(minfs_sb->inode_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    uint32_t byte_index = inode_num / 8;
    uint32_t bit_index = inode_num % 8;
    
    // Check if the inode is already free
    if (!(bitmap[byte_index] & (1 << bit_index))) {
        return 0; // Already free
    }
    
    // Mark the inode as free
    bitmap[byte_index] &= ~(1 << bit_index);
    
    // Write the updated bitmap
    if (minfs_write_block(minfs_sb->inode_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    // Update superblock
    minfs_sb->free_inodes++;
    
    // Write the updated superblock
    uint8_t sb_buffer[MINFS_BLOCK_SIZE];
    memcpy(sb_buffer, minfs_sb, sizeof(minfs_superblock_t));
    if (minfs_write_block(0, sb_buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    return 0;
}

/* Free a block */
static int minfs_free_block(uint32_t block_num) {
    if (!minfs_sb || block_num < minfs_sb->data_block_start || 
        block_num >= minfs_sb->data_block_start + minfs_sb->block_count) {
        return -1;
    }
    
    uint32_t block_index = block_num - minfs_sb->data_block_start;
    
    uint8_t bitmap[MINFS_BLOCK_SIZE];
    if (minfs_read_block(minfs_sb->block_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    uint32_t byte_index = block_index / 8;
    uint32_t bit_index = block_index % 8;
    
    // Check if the block is already free
    if (!(bitmap[byte_index] & (1 << bit_index))) {
        return 0; // Already free
    }
    
    // Mark the block as free
    bitmap[byte_index] &= ~(1 << bit_index);
    
    // Write the updated bitmap
    if (minfs_write_block(minfs_sb->block_bitmap_block, bitmap) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    // Update superblock
    minfs_sb->free_blocks++;
    
    // Write the updated superblock
    uint8_t sb_buffer[MINFS_BLOCK_SIZE];
    memcpy(sb_buffer, minfs_sb, sizeof(minfs_superblock_t));
    if (minfs_write_block(0, sb_buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    return 0;
}

/* Initialize MinFS */
void minfs_init() {
    terminal_writestring("Initializing MinFS file system...\n");
    
    // Allocate memory for the superblock
    minfs_sb = (minfs_superblock_t*)kmalloc(sizeof(minfs_superblock_t));
    if (!minfs_sb) {
        terminal_writestring("Failed to allocate memory for MinFS superblock\n");
        return;
    }
    
    terminal_writestring("MinFS initialized\n");
}

/* Create a MinFS file system on a device */
int minfs_format(fs_node_t* device, const char* volume_name) {
    if (!device) {
        return -1;
    }
    
    // Calculate file system parameters
    uint32_t device_size = device->length;
    uint32_t block_count = device_size / MINFS_BLOCK_SIZE;
    
    // Minimum blocks needed for a valid file system
    if (block_count < 10) {
        return -1; // Device too small
    }
    
    // Calculate inode count (1 inode per 4 blocks, minimum 100)
    uint32_t inode_count = block_count / 4;
    if (inode_count < 100) {
        inode_count = 100;
    }
    
    // Calculate inode table size in blocks
    uint32_t inode_table_size = (inode_count * sizeof(minfs_inode_t) + MINFS_BLOCK_SIZE - 1) / MINFS_BLOCK_SIZE;
    
    // Calculate bitmap sizes in blocks
    uint32_t inode_bitmap_size = (inode_count + 8 * MINFS_BLOCK_SIZE - 1) / (8 * MINFS_BLOCK_SIZE);
    uint32_t block_bitmap_size = (block_count + 8 * MINFS_BLOCK_SIZE - 1) / (8 * MINFS_BLOCK_SIZE);
    
    // Calculate journal size (5% of blocks, minimum 10)
    uint32_t journal_size = block_count / 20;
    if (journal_size < 10) {
        journal_size = 10;
    }
    
    // Calculate data block start
    uint32_t data_block_start = 1 + inode_bitmap_size + block_bitmap_size + inode_table_size + journal_size;
    
    // Check if we have enough space for data blocks
    if (data_block_start >= block_count) {
        return -1; // Not enough space
    }
    
    // Create superblock
    minfs_superblock_t sb;
    memset(&sb, 0, sizeof(minfs_superblock_t));
    
    sb.magic = MINFS_MAGIC;
    sb.version = MINFS_VERSION;
    sb.block_size = MINFS_BLOCK_SIZE;
    sb.inode_count = inode_count;
    sb.block_count = block_count - data_block_start;
    sb.free_inodes = inode_count - 1; // Root directory uses one inode
    sb.free_blocks = sb.block_count - 1; // Root directory uses one block
    sb.inode_bitmap_block = 1;
    sb.block_bitmap_block = 1 + inode_bitmap_size;
    sb.inode_table_block = 1 + inode_bitmap_size + block_bitmap_size;
    sb.data_block_start = data_block_start;
    sb.journal_block = 1 + inode_bitmap_size + block_bitmap_size + inode_table_size;
    sb.journal_size = journal_size;
    
    // Copy volume name
    strncpy((char*)sb.name, volume_name, 15);
    sb.name[15] = '\0';
    
    // Write superblock to device
    uint8_t buffer[MINFS_BLOCK_SIZE];
    memset(buffer, 0, MINFS_BLOCK_SIZE);
    memcpy(buffer, &sb, sizeof(minfs_superblock_t));
    
    if (vfs_write(device, 0, MINFS_BLOCK_SIZE, buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    // Initialize inode bitmap (mark inode 0 as used for root directory)
    memset(buffer, 0, MINFS_BLOCK_SIZE);
    buffer[0] = 0x01; // Mark inode 0 as used
    
    for (uint32_t i = 0; i < inode_bitmap_size; i++) {
        if (vfs_write(device, (sb.inode_bitmap_block + i) * MINFS_BLOCK_SIZE, 
                     MINFS_BLOCK_SIZE, i == 0 ? buffer : (uint8_t*)"\0") != MINFS_BLOCK_SIZE) {
            return -1;
        }
    }
    
    // Initialize block bitmap (mark first data block as used for root directory)
    memset(buffer, 0, MINFS_BLOCK_SIZE);
    buffer[0] = 0x01; // Mark first data block as used
    
    for (uint32_t i = 0; i < block_bitmap_size; i++) {
        if (vfs_write(device, (sb.block_bitmap_block + i) * MINFS_BLOCK_SIZE, 
                     MINFS_BLOCK_SIZE, i == 0 ? buffer : (uint8_t*)"\0") != MINFS_BLOCK_SIZE) {
            return -1;
        }
    }
    
    // Initialize inode table
    memset(buffer, 0, MINFS_BLOCK_SIZE);
    
    // Create root directory inode
    minfs_inode_t root_inode;
    memset(&root_inode, 0, sizeof(minfs_inode_t));
    
    root_inode.mode = MINFS_TYPE_DIRECTORY | 0755; // drwxr-xr-x
    root_inode.uid = 0; // root user
    root_inode.gid = 0; // root group
    root_inode.size = MINFS_BLOCK_SIZE;
    root_inode.atime = root_inode.mtime = root_inode.ctime = 0; // Current time (to be implemented)
    root_inode.links_count = 2; // . and ..
    root_inode.blocks[0] = data_block_start; // First data block
    
    memcpy(buffer, &root_inode, sizeof(minfs_inode_t));
    
    if (vfs_write(device, sb.inode_table_block * MINFS_BLOCK_SIZE, MINFS_BLOCK_SIZE, buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    // Initialize remaining inode table blocks
    memset(buffer, 0, MINFS_BLOCK_SIZE);
    
    for (uint32_t i = 1; i < inode_table_size; i++) {
        if (vfs_write(device, (sb.inode_table_block + i) * MINFS_BLOCK_SIZE, 
                     MINFS_BLOCK_SIZE, buffer) != MINFS_BLOCK_SIZE) {
            return -1;
        }
    }
    
    // Initialize journal
    memset(buffer, 0, MINFS_BLOCK_SIZE);
    
    for (uint32_t i = 0; i < journal_size; i++) {
        if (vfs_write(device, (sb.journal_block + i) * MINFS_BLOCK_SIZE, 
                     MINFS_BLOCK_SIZE, buffer) != MINFS_BLOCK_SIZE) {
            return -1;
        }
    }
    
    // Initialize root directory
    memset(buffer, 0, MINFS_BLOCK_SIZE);
    
    // Create . entry
    minfs_dirent_t* entry = (minfs_dirent_t*)buffer;
    entry->inode = 0; // Root inode
    entry->rec_len = 12; // 8 bytes header + 1 byte name + 3 bytes padding
    entry->name_len = 1;
    entry->file_type = MINFS_TYPE_DIRECTORY;
    entry->name[0] = '.';
    
    // Create .. entry
    entry = (minfs_dirent_t*)(buffer + 12);
    entry->inode = 0; // Root inode (parent of root is root)
    entry->rec_len = MINFS_BLOCK_SIZE - 12; // Remainder of block
    entry->name_len = 2;
    entry->file_type = MINFS_TYPE_DIRECTORY;
    entry->name[0] = '.';
    entry->name[1] = '.';
    
    if (vfs_write(device, data_block_start * MINFS_BLOCK_SIZE, MINFS_BLOCK_SIZE, buffer) != MINFS_BLOCK_SIZE) {
        return -1;
    }
    
    return 0;
}

/* VFS operations for MinFS */

/* Read from a MinFS file */
static uint32_t minfs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // Implementation to be added
    return 0;
}

/* Write to a MinFS file */
static uint32_t minfs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // Implementation to be added
    return 0;
}

/* Open a MinFS file */
static void minfs_open(fs_node_t* node) {
    // Implementation to be added
}

/* Close a MinFS file */
static void minfs_close(fs_node_t* node) {
    // Implementation to be added
}

/* Read a MinFS directory entry */
static dirent_t* minfs_readdir(fs_node_t* node, uint32_t index) {
    // Implementation to be added
    return NULL;
}

/* Find a file in a MinFS directory */
static fs_node_t* minfs_finddir(fs_node_t* node, char* name) {
    // Implementation to be added
    return NULL;
}

/* Create a file or directory in MinFS */
static void minfs_create(fs_node_t* node, char* name, uint16_t permission) {
    // Implementation to be added
}

/* Delete a file or directory in MinFS */
static void minfs_unlink(fs_node_t* node, char* name) {
    // Implementation to be added
}

/* Mount a MinFS file system */
fs_node_t* minfs_mount(fs_node_t* device) {
    if (!device) {
        return NULL;
    }
    
    // Save the device node
    minfs_device = device;
    
    // Read the superblock
    uint8_t buffer[MINFS_BLOCK_SIZE];
    if (vfs_read(device, 0, MINFS_BLOCK_SIZE, buffer) != MINFS_BLOCK_SIZE) {
        return NULL;
    }
    
    // Copy superblock to in-memory structure
    memcpy(minfs_sb, buffer, sizeof(minfs_superblock_t));
    
    // Verify magic number
    if (minfs_sb->magic != MINFS_MAGIC) {
        return NULL; // Not a MinFS file system
    }
    
    // Create root node
    fs_node_t* root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    memset(root, 0, sizeof(fs_node_t));
    
    strcpy(root->name, "/");
    root->mask = 0755;
    root->uid = 0;
    root->gid = 0;
    root->flags = VFS_DIRECTORY;
    root->inode = 0;
    root->length = 0;
    root->impl = 0;
    
    // Set up operations
    root->read = minfs_read;
    root->write = minfs_write;
    root->open = minfs_open;
    root->close = minfs_close;<response clipped><NOTE>To save on context only part of this file has been shown to you. You should retry this tool after you have searched inside the file with `grep -n` in order to find the line numbers of what you are looking for.</NOTE>