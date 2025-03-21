#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

/* Maximum command length */
#define SHELL_MAX_COMMAND_LENGTH 256

/* Maximum number of arguments */
#define SHELL_MAX_ARGS 32

/* Maximum length of command history */
#define SHELL_HISTORY_SIZE 50

/* Shell command structure */
typedef struct {
    const char* name;
    const char* description;
    int (*function)(int argc, char** argv);
} shell_command_t;

/* Initialize the shell */
void shell_init(void);

/* Run the shell */
void shell_run(void);

/* Parse a command line */
int shell_parse_command(char* command, char** argv);

/* Execute a command */
int shell_execute(int argc, char** argv);

/* Register a command */
int shell_register_command(const char* name, const char* description, int (*function)(int argc, char** argv));

/* Built-in commands */
int shell_cmd_help(int argc, char** argv);
int shell_cmd_echo(int argc, char** argv);
int shell_cmd_exit(int argc, char** argv);
int shell_cmd_clear(int argc, char** argv);
int shell_cmd_ls(int argc, char** argv);
int shell_cmd_cd(int argc, char** argv);
int shell_cmd_cat(int argc, char** argv);
int shell_cmd_mkdir(int argc, char** argv);
int shell_cmd_rm(int argc, char** argv);
int shell_cmd_rmdir(int argc, char** argv);
int shell_cmd_touch(int argc, char** argv);
int shell_cmd_ps(int argc, char** argv);
int shell_cmd_kill(int argc, char** argv);
int shell_cmd_ifconfig(int argc, char** argv);
int shell_cmd_ping(int argc, char** argv);
int shell_cmd_netstat(int argc, char** argv);

#endif /* SHELL_H */
