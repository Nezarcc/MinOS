#ifndef INIT_H
#define INIT_H

#include <stdint.h>

/* Initialize the system */
void init_system(void);

/* Mount the root file system */
int init_mount_root(void);

/* Load and execute init process */
int init_exec(void);

#endif /* INIT_H */
