#include "vfs.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Root file system node */
static fs_node_t* fs_root = NULL;

/* Initialize the VFS */
void vfs_init() {
    terminal_writestring("Initializing Virtual File System...\n");
    
    // Root node will be set when a file system is mounted
    fs_root = NULL;
    
    terminal_writestring("VFS initialized\n");
}

/* Get the root node */
fs_node_t* vfs_get_root() {
    return fs_root;
}

/* Mount a file system */
int vfs_mount(char* path, fs_node_t* node) {
    // For now, we only support mounting at root
    if (strcmp(path, "/") == 0) {
        fs_root = node;
        return 0;
    }
    
    // TODO: Implement mounting at arbitrary paths
    return -1;
}

/* Read from a file */
uint32_t vfs_read(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // Check if the node has a read function
    if (node->read != 0) {
        return node->read(node, offset, size, buffer);
    }
    return 0;
}

/* Write to a file */
uint32_t vfs_write(fs_node_t* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    // Check if the node has a write function
    if (node->write != 0) {
        return node->write(node, offset, size, buffer);
    }
    return 0;
}

/* Open a file */
void vfs_open(fs_node_t* node, uint8_t read, uint8_t write) {
    // Check if the node has an open function
    if (node->open != 0) {
        node->open(node);
    }
}

/* Close a file */
void vfs_close(fs_node_t* node) {
    // Check if the node has a close function
    if (node->close != 0) {
        node->close(node);
    }
}

/* Read a directory entry */
dirent_t* vfs_readdir(fs_node_t* node, uint32_t index) {
    // Check if the node is a directory and has a readdir function
    if ((node->flags & VFS_DIRECTORY) && node->readdir != 0) {
        return node->readdir(node, index);
    }
    return NULL;
}

/* Find a file in a directory */
fs_node_t* vfs_finddir(fs_node_t* node, char* name) {
    // Check if the node is a directory and has a finddir function
    if ((node->flags & VFS_DIRECTORY) && node->finddir != 0) {
        return node->finddir(node, name);
    }
    return NULL;
}

/* Create a file or directory */
void vfs_create(fs_node_t* node, char* name, uint16_t permission) {
    // Check if the node is a directory and has a create function
    if ((node->flags & VFS_DIRECTORY) && node->create != 0) {
        node->create(node, name, permission);
    }
}

/* Delete a file or directory */
void vfs_unlink(fs_node_t* node, char* name) {
    // Check if the node is a directory and has an unlink function
    if ((node->flags & VFS_DIRECTORY) && node->unlink != 0) {
        node->unlink(node, name);
    }
}
