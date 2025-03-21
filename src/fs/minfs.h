#ifndef MINFS_H
#define MINFS_H

#include "vfs.h"
#include <stdint.h>
#include <stddef.h>

/* MinFS superblock structure */
typedef struct {
    uint32_t magic;              /* Magic number to identify MinFS */
    uint32_t version;            /* File system version */
    uint32_t block_size;         /* Size of blocks in bytes */
    uint32_t inode_count;        /* Total number of inodes */
    uint32_t block_count;        /* Total number of blocks */
    uint32_t free_inodes;        /* Number of free inodes */
    uint32_t free_blocks;        /* Number of free blocks */
    uint32_t inode_bitmap_block; /* Block containing inode bitmap */
    uint32_t block_bitmap_block; /* Block containing block bitmap */
    uint32_t inode_table_block;  /* First block of inode table */
    uint32_t data_block_start;   /* First data block */
    uint32_t journal_block;      /* First block of journal */
    uint32_t journal_size;       /* Size of journal in blocks */
    uint8_t  name[16];           /* Volume name */
    uint8_t  reserved[476];      /* Padding to make superblock 512 bytes */
} minfs_superblock_t;

/* MinFS inode structure */
typedef struct {
    uint32_t mode;               /* File type and permissions */
    uint32_t uid;                /* User ID */
    uint32_t gid;                /* Group ID */
    uint32_t size;               /* File size in bytes */
    uint32_t atime;              /* Last access time */
    uint32_t mtime;              /* Last modification time */
    uint32_t ctime;              /* Creation time */
    uint32_t dtime;              /* Deletion time (0 if not deleted) */
    uint16_t links_count;        /* Hard links count */
    uint16_t flags;              /* File flags */
    uint32_t blocks[12];         /* Direct block pointers */
    uint32_t indirect_block;     /* Indirect block pointer */
    uint32_t double_indirect;    /* Double indirect block pointer */
    uint32_t triple_indirect;    /* Triple indirect block pointer */
    uint8_t  reserved[28];       /* Padding to make inode 128 bytes */
} minfs_inode_t;

/* MinFS directory entry structure */
typedef struct {
    uint32_t inode;              /* Inode number */
    uint16_t rec_len;            /* Record length */
    uint8_t  name_len;           /* Name length */
    uint8_t  file_type;          /* File type */
    char     name[0];            /* File name (variable length) */
} minfs_dirent_t;

/* MinFS journal entry types */
#define MINFS_JOURNAL_START    0x01
#define MINFS_JOURNAL_COMMIT   0x02
#define MINFS_JOURNAL_BLOCK    0x03
#define MINFS_JOURNAL_INODE    0x04

/* MinFS journal header */
typedef struct {
    uint32_t magic;              /* Journal magic number */
    uint32_t sequence;           /* Sequence number */
    uint32_t type;               /* Entry type */
    uint32_t size;               /* Entry size in bytes */
} minfs_journal_header_t;

/* Initialize MinFS */
void minfs_init(void);

/* Create a MinFS file system on a device */
int minfs_format(fs_node_t* device, const char* volume_name);

/* Mount a MinFS file system */
fs_node_t* minfs_mount(fs_node_t* device);

#endif /* MINFS_H */
