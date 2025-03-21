#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

/* System call numbers */
#define SYS_EXIT        1
#define SYS_FORK        2
#define SYS_READ        3
#define SYS_WRITE       4
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_WAITPID     7
#define SYS_EXEC        8
#define SYS_GETPID      9
#define SYS_SLEEP      10
#define SYS_KILL       11
#define SYS_MMAP       12
#define SYS_MUNMAP     13
#define SYS_STAT       14
#define SYS_MKDIR      15
#define SYS_RMDIR      16
#define SYS_CHDIR      17
#define SYS_GETCWD     18
#define SYS_TIME       19
#define SYS_CHMOD      20

/* Initialize system call interface */
void syscall_init(void);

/* System call handler */
void syscall_handler(void);

/* Register a system call handler */
void register_syscall(uint32_t num, void* handler);

#endif /* SYSCALL_H */
