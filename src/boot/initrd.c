#include "initrd.h"
#include "../fs/vfs.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <string.h>

/* Initial ramdisk magic number */
#define INITRD_MAGIC 0x52444E49 /* "INRD" */

/* Initial ramdisk version */
#define INITRD_VERSION 0x0001

/* Initial ramdisk data */
static uint32_t initrd_location = 0;
static initrd_header_t* initrd_header = NULL;
static initrd_file_header_t* file_headers = NULL;
static fs_node_t* initrd_root = NULL;
static fs_node_t* initrd_dev = NULL;

/* Read from an initrd file */
static uint32_t initrd_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    initrd_file_header_t header = file_headers[node->inode];
    
    // Check if offset is beyond file size
    if (offset >= header.length) {
        return 0;
    }
    
    // Calculate how much to read
    uint32_t read_size = size;
    if (offset + size > header.length) {
        read_size = header.length - offset;
    }
    
    // Copy data from the ramdisk
    memcpy(buffer, (uint8_t*)(initrd_location + header.offset + offset), read_size);
    
    return read_size;
}

/* Read directory entries from initrd */
static dirent_t* initrd_readdir(fs_node_t* node, uint32_t index) {
    // Check if this is a directory
    if (!(node->flags & VFS_DIRECTORY)) {
        return NULL;
    }
    
    // Root directory
    if (node == initrd_root) {
        // Check if index is valid
        if (index >= initrd_header->num_files) {
            return NULL;
        }
        
        // Create a directory entry
        static dirent_t dirent;
        strcpy(dirent.name, (char*)file_headers[index].name);
        dirent.inode = index;
        
        return &dirent;
    }
    
    return NULL;
}

/* Find a file in the initrd */
static fs_node_t* initrd_finddir(fs_node_t* node, char* name) {
    // Check if this is a directory
    if (!(node->flags & VFS_DIRECTORY)) {
        return NULL;
    }
    
    // Root directory
    if (node == initrd_root) {
        // Search for the file
        for (uint32_t i = 0; i < initrd_header->num_files; i++) {
            if (strcmp(name, (char*)file_headers[i].name) == 0) {
                // Create a file node
                fs_node_t* file_node = (fs_node_t*)kmalloc(sizeof(fs_node_t));
                strcpy(file_node->name, name);
                file_node->mask = 0444; // Read-only
                file_node->uid = 0;
                file_node->gid = 0;
                file_node->flags = VFS_FILE;
                file_node->inode = i;
                file_node->length = file_headers[i].length;
                file_node->read = initrd_read;
                file_node->write = NULL; // Read-only
                file_node->open = NULL;
                file_node->close = NULL;
                file_node->readdir = NULL;
                file_node->finddir = NULL;
                file_node->impl = 0;
                
                return file_node;
            }
        }
    }
    
    return NULL;
}

/* Initialize the initial ramdisk */
fs_node_t* initrd_init(uint32_t location) {
    terminal_writestring("Initializing initial ramdisk...\n");
    
    // Save the location
    initrd_location = location;
    
    // Get the header
    initrd_header = (initrd_header_t*)location;
    
    // Verify the magic number
    if (initrd_header->magic != INITRD_MAGIC) {
        terminal_writestring("Invalid initial ramdisk: bad magic number\n");
        return NULL;
    }
    
    // Verify the version
    if (initrd_header->version != INITRD_VERSION) {
        terminal_writestring("Invalid initial ramdisk: unsupported version\n");
        return NULL;
    }
    
    // Get the file headers
    file_headers = (initrd_file_header_t*)(location + sizeof(initrd_header_t));
    
    // Create the root directory node
    initrd_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
    strcpy(initrd_root->name, "initrd");
    initrd_root->mask = 0555; // Read and execute
    initrd_root->uid = 0;
    initrd_root->gid = 0;
    initrd_root->flags = VFS_DIRECTORY;
    initrd_root->inode = 0;
    initrd_root->length = 0;
    initrd_root->read = NULL;
    initrd_root->write = NULL;
    initrd_root->open = NULL;
    initrd_root->close = NULL;
    initrd_root->readdir = initrd_readdir;
    initrd_root->finddir = initrd_finddir;
    initrd_root->impl = 0;
    
    terminal_writestring("Initial ramdisk initialized with ");
    // Print number of files
    terminal_writestring(" files\n");
    
    return initrd_root;
}

/* Create an initial ramdisk from a directory */
int initrd_create(const char* directory, const char* output_file) {
    // This function would be implemented in a host tool, not in the kernel
    return -1;
}
