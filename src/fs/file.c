#include "file.h"
#include "vfs.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Maximum number of open files */
#define MAX_OPEN_FILES 256

/* File descriptor table */
static file_descriptor_t* fd_table[MAX_OPEN_FILES];

/* Current working directory */
static char current_dir[256] = "/";

/* Initialize file system interface */
void file_init() {
    terminal_writestring("Initializing file system interface...\n");
    
    // Initialize file descriptor table
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        fd_table[i] = NULL;
    }
    
    // Set up standard file descriptors (stdin, stdout, stderr)
    // To be implemented when we have device files
    
    terminal_writestring("File system interface initialized\n");
}

/* Allocate a file descriptor */
static int alloc_fd() {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (fd_table[i] == NULL) {
            return i;
        }
    }
    return -1; // No free file descriptors
}

/* Resolve a path to a VFS node */
static fs_node_t* resolve_path(const char* path) {
    if (!path) {
        return NULL;
    }
    
    // Get the root node
    fs_node_t* root = vfs_get_root();
    if (!root) {
        return NULL; // No root file system
    }
    
    // Handle absolute paths
    if (path[0] == '/') {
        // Skip leading slash
        path++;
        
        // Empty path means root directory
        if (path[0] == '\0') {
            return root;
        }
    } else {
        // Relative path - start from current directory
        // To be implemented
        return NULL;
    }
    
    // Parse path components
    fs_node_t* current = root;
    char component[256];
    int i = 0;
    
    while (*path) {
        // Extract next path component
        i = 0;
        while (*path && *path != '/') {
            component[i++] = *path++;
        }
        component[i] = '\0';
        
        // Skip consecutive slashes
        while (*path == '/') {
            path++;
        }
        
        // Find the component in the current directory
        current = vfs_finddir(current, component);
        if (!current) {
            return NULL; // Component not found
        }
        
        // If we've reached the end of the path, return the node
        if (*path == '\0') {
            return current;
        }
        
        // Make sure the current node is a directory
        if (!(current->flags & VFS_DIRECTORY)) {
            return NULL; // Not a directory
        }
    }
    
    return current;
}

/* Open a file */
int file_open(const char* path, uint32_t flags) {
    // Resolve the path to a VFS node
    fs_node_t* node = resolve_path(path);
    
    // If the file doesn't exist and O_CREAT is specified, create it
    if (!node && (flags & O_CREAT)) {
        // Extract the directory and filename
        char dir_path[256];
        char filename[256];
        
        // Find the last slash in the path
        const char* last_slash = strrchr(path, '/');
        if (!last_slash) {
            // No slash, use current directory
            strcpy(dir_path, ".");
            strcpy(filename, path);
        } else {
            // Copy directory path
            int dir_len = last_slash - path;
            strncpy(dir_path, path, dir_len);
            dir_path[dir_len] = '\0';
            
            // Copy filename
            strcpy(filename, last_slash + 1);
        }
        
        // Resolve the directory
        fs_node_t* dir = resolve_path(dir_path);
        if (!dir) {
            return -1; // Directory not found
        }
        
        // Create the file
        vfs_create(dir, filename, 0644); // rw-r--r--
        
        // Try to resolve the path again
        node = resolve_path(path);
        if (!node) {
            return -1; // Failed to create file
        }
    }
    
    // If the file doesn't exist, return error
    if (!node) {
        return -1;
    }
    
    // Check if the file is a directory and O_DIRECTORY is not specified
    if ((node->flags & VFS_DIRECTORY) && !(flags & O_DIRECTORY)) {
        return -1; // Cannot open directory as file
    }
    
    // Allocate a file descriptor
    int fd = alloc_fd();
    if (fd < 0) {
        return -1; // No free file descriptors
    }
    
    // Allocate a file descriptor structure
    file_descriptor_t* file = (file_descriptor_t*)kmalloc(sizeof(file_descriptor_t));
    if (!file) {
        return -1; // Out of memory
    }
    
    // Initialize the file descriptor
    file->node = node;
    file->offset = 0;
    file->flags = flags;
    file->refcount = 1;
    
    // If O_APPEND is specified, seek to the end of the file
    if (flags & O_APPEND) {
        file->offset = node->length;
    }
    
    // If O_TRUNC is specified, truncate the file
    if (flags & O_TRUNC) {
        // To be implemented
    }
    
    // Open the file
    vfs_open(node, flags & O_RDONLY, flags & O_WRONLY);
    
    // Store the file descriptor
    fd_table[fd] = file;
    
    return fd;
}

/* Close a file */
int file_close(int fd) {
    // Check if the file descriptor is valid
    if (fd < 0 || fd >= MAX_OPEN_FILES || fd_table[fd] == NULL) {
        return -1;
    }
    
    // Get the file descriptor
    file_descriptor_t* file = fd_table[fd];
    
    // Decrement the reference count
    file->refcount--;
    
    // If the reference count is zero, close the file
    if (file->refcount == 0) {
        // Close the file
        vfs_close(file->node);
        
        // Free the file descriptor
        kfree(file);
    }
    
    // Clear the file descriptor table entry
    fd_table[fd] = NULL;
    
    return 0;
}

/* Read from a file */
int file_read(int fd, void* buffer, uint32_t size) {
    // Check if the file descriptor is valid
    if (fd < 0 || fd >= MAX_OPEN_FILES || fd_table[fd] == NULL) {
        return -1;
    }
    
    // Get the file descriptor
    file_descriptor_t* file = fd_table[fd];
    
    // Check if the file is readable
    if ((file->flags & O_WRONLY) && !(file->flags & O_RDWR)) {
        return -1; // File is not readable
    }
    
    // Read from the file
    uint32_t bytes_read = vfs_read(file->node, file->offset, size, (uint8_t*)buffer);
    
    // Update the file offset
    file->offset += bytes_read;
    
    return bytes_read;
}

/* Write to a file */
int file_write(int fd, const void* buffer, uint32_t size) {
    // Check if the file descriptor is valid
    if (fd < 0 || fd >= MAX_OPEN_FILES || fd_table[fd] == NULL) {
        return -1;
    }
    
    // Get the file descriptor
    file_descriptor_t* file = fd_table[fd];
    
    // Check if the file is writable
    if ((file->flags & O_RDONLY) && !(file->flags & O_RDWR)) {
        return -1; // File is not writable
    }
    
    // Write to the file
    uint32_t bytes_written = vfs_write(file->node, file->offset, size, (uint8_t*)buffer);
    
    // Update the file offset
    file->offset += bytes_written;
    
    return bytes_written;
}

/* Seek within a file */
int file_seek(int fd, int offset, int whence) {
    // Check if the file descriptor is valid
    if (fd < 0 || fd >= MAX_OPEN_FILES || fd_table[fd] == NULL) {
        return -1;
    }
    
    // Get the file descriptor
    file_descriptor_t* file = fd_table[fd];
    
    // Calculate the new offset
    switch (whence) {
        case VFS_SEEK_SET:
            file->offset = offset;
            break;
        case VFS_SEEK_CUR:
            file->offset += offset;
            break;
        case VFS_SEEK_END:
            file->offset = file->node->length + offset;
            break;
        default:
            return -1; // Invalid whence
    }
    
    // Make sure the offset is not negative
    if (file->offset < 0) {
        file->offset = 0;
    }
    
    return file->offset;
}

/* Get file status */
int file_stat(const char* path, void* stat_buf) {
    // Resolve the path to a VFS node
    fs_node_t* node = resolve_path(path);
    if (!node) {
        return -1; // File not found
    }
    
    // To be implemented
    return 0;
}

/* Create a directory */
int file_mkdir(const char* path, uint32_t mode) {
    // Extract the directory and filename
    char dir_path[256];
    char dirname[256];
    
    // Find the last slash in the path
    const char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        // No slash, use current directory
        strcpy(dir_path, ".");
        strcpy(dirname, path);
    } else {
        // Copy directory path
        int dir_len = last_slash - path;
        strncpy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';
        
        // Copy dirname
        strcpy(dirname, last_slash + 1);
    }
    
    // Resolve the directory
    fs_node_t* dir = resolve_path(dir_path);
    if (!dir) {
        return -1; // Directory not found
    }
    
    // Create the directory
    vfs_create(dir, dirname, mode | VFS_DIRECTORY);
    
    return 0;
}

/* Remove a file */
int file_unlink(const char* path) {
    // Extract the directory and filename
    char dir_path[256];
    char filename[256];
    
    // Find the last slash in the path
    const char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        // No slash, use current directory
        strcpy(dir_path, ".");
        strcpy(filename, path);
    } else {
        // Copy directory path
        int dir_len = last_slash - path;
        strncpy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';
        
        // Copy filename
        strcpy(filename, last_slash + 1);
    }
    
    // Resolve the directory
    fs_node_t* dir = resolve_path(dir_path);
    if (!dir) {
        return -1; // Directory not found
    }
    
    // Remove the file
    vfs_unlink(dir, filename);
    
    return 0;
}

/* Remove a directory */
int file_rmdir(const char* path) {
    // Same as unlink for now
    return file_unlink(path);
}

/* Change current directory */
int file_chdir(const char* path) {
    // Resolve the path to a VFS node
    fs_node_t* node = resolve_path(path);
    if (!node) {
        return -1; // Directory not found
    }
    
    // Make sure the node is a directory
    if (!(node->flags & VFS_DIRECTORY)) {
        return -1; // Not a directory
    }
    
    // Update the current directory
    if (path[0] == '/') {
        // Absolute path
        strcpy(current_dir, path);
    } else {
        // Relative path
        // To be implemented
    }
    
    return 0;
}

/* Get current directory */
char* file_getcwd(char* buf, uint32_t size) {
    if (size < strlen(current_dir) + 1) {
        return NULL; // Buffer too small
    }
    
    strcpy(buf, current_dir);
    return buf;
}
