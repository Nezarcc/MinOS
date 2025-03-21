#ifndef INITRD_H
#define INITRD_H

#include "../fs/vfs.h"
#include <stdint.h>

/* Initial ramdisk header */
typedef struct {
    uint32_t magic;       /* Magic number to identify initrd */
    uint32_t version;     /* Version of the initrd format */
    uint32_t num_files;   /* Number of files in the initrd */
    uint32_t size;        /* Total size of the initrd */
} initrd_header_t;

/* Initial ramdisk file header */
typedef struct {
    uint8_t name[64];     /* Filename (null-terminated) */
    uint32_t offset;      /* Offset of file data from start of initrd */
    uint32_t length;      /* Length of file in bytes */
} initrd_file_header_t;

/* Initialize the initial ramdisk */
fs_node_t* initrd_init(uint32_t location);

/* Create an initial ramdisk from a directory */
int initrd_create(const char* directory, const char* output_file);

#endif /* INITRD_H */
