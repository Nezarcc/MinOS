#include "shell.h"
#include "../kernel/kernel.h"
#include "../fs/file.h"
#include "../net/network.h"
#include <stdint.h>
#include <string.h>

/* Command table */
static shell_command_t* command_table[64];
static uint32_t command_count = 0;

/* Command history */
static char command_history[SHELL_HISTORY_SIZE][SHELL_MAX_COMMAND_LENGTH];
static uint32_t history_count = 0;
static uint32_t history_index = 0;

/* Current working directory */
static char current_dir[256] = "/";

/* Shell prompt */
static char prompt[32] = "minos> ";

/* Initialize the shell */
void shell_init() {
    terminal_writestring("Initializing MinOS shell...\n");
    
    // Clear command table
    for (int i = 0; i < 64; i++) {
        command_table[i] = NULL;
    }
    
    // Register built-in commands
    shell_register_command("help", "Display help information", shell_cmd_help);
    shell_register_command("echo", "Display a line of text", shell_cmd_echo);
    shell_register_command("exit", "Exit the shell", shell_cmd_exit);
    shell_register_command("clear", "Clear the terminal screen", shell_cmd_clear);
    shell_register_command("ls", "List directory contents", shell_cmd_ls);
    shell_register_command("cd", "Change the current directory", shell_cmd_cd);
    shell_register_command("cat", "Concatenate and display file contents", shell_cmd_cat);
    shell_register_command("mkdir", "Create a directory", shell_cmd_mkdir);
    shell_register_command("rm", "Remove a file", shell_cmd_rm);
    shell_register_command("rmdir", "Remove a directory", shell_cmd_rmdir);
    shell_register_command("touch", "Create an empty file", shell_cmd_touch);
    shell_register_command("ps", "Report process status", shell_cmd_ps);
    shell_register_command("kill", "Terminate a process", shell_cmd_kill);
    shell_register_command("ifconfig", "Configure network interfaces", shell_cmd_ifconfig);
    shell_register_command("ping", "Send ICMP ECHO_REQUEST to network hosts", shell_cmd_ping);
    shell_register_command("netstat", "Print network connections", shell_cmd_netstat);
    
    // Clear command history
    for (int i = 0; i < SHELL_HISTORY_SIZE; i++) {
        command_history[i][0] = '\0';
    }
    
    terminal_writestring("MinOS shell initialized\n");
}

/* Run the shell */
void shell_run() {
    char command[SHELL_MAX_COMMAND_LENGTH];
    char* argv[SHELL_MAX_ARGS];
    int argc;
    
    // Main shell loop
    while (1) {
        // Display prompt
        terminal_writestring(prompt);
        
        // Read command
        int i = 0;
        char c;
        while (i < SHELL_MAX_COMMAND_LENGTH - 1) {
            // In a real implementation, this would read from the keyboard
            // For now, we'll just simulate a command
            if (i == 0) {
                strcpy(command, "help");
                i = 4;
                break;
            }
            
            // Check for newline or carriage return
            if (c == '\n' || c == '\r') {
                break;
            }
            
            // Add character to command
            command[i++] = c;
        }
        command[i] = '\0';
        
        // Add command to history
        if (i > 0) {
            strcpy(command_history[history_index], command);
            history_index = (history_index + 1) % SHELL_HISTORY_SIZE;
            if (history_count < SHELL_HISTORY_SIZE) {
                history_count++;
            }
        }
        
        // Parse command
        argc = shell_parse_command(command, argv);
        
        // Execute command
        if (argc > 0) {
            shell_execute(argc, argv);
        }
    }
}

/* Parse a command line */
int shell_parse_command(char* command, char** argv) {
    int argc = 0;
    char* token;
    char* rest = command;
    
    // Skip leading whitespace
    while (*rest == ' ' || *rest == '\t') {
        rest++;
    }
    
    // Parse command into arguments
    while ((token = strtok_r(rest, " \t", &rest)) != NULL && argc < SHELL_MAX_ARGS) {
        argv[argc++] = token;
    }
    
    // Null-terminate argument list
    argv[argc] = NULL;
    
    return argc;
}

/* Execute a command */
int shell_execute(int argc, char** argv) {
    if (argc == 0) {
        return 0;
    }
    
    // Look up command in command table
    for (uint32_t i = 0; i < command_count; i++) {
        if (strcmp(command_table[i]->name, argv[0]) == 0) {
            // Execute command
            return command_table[i]->function(argc, argv);
        }
    }
    
    // Command not found
    terminal_writestring("minos: command not found: ");
    terminal_writestring(argv[0]);
    terminal_writestring("\n");
    
    return -1;
}

/* Register a command */
int shell_register_command(const char* name, const char* description, int (*function)(int argc, char** argv)) {
    if (command_count >= 64) {
        return -1; // Command table full
    }
    
    // Allocate command structure
    shell_command_t* command = (shell_command_t*)kmalloc(sizeof(shell_command_t));
    if (!command) {
        return -1; // Out of memory
    }
    
    // Initialize command
    command->name = name;
    command->description = description;
    command->function = function;
    
    // Add to command table
    command_table[command_count++] = command;
    
    return 0;
}

/* Built-in command: help */
int shell_cmd_help(int argc, char** argv) {
    if (argc > 1) {
        // Display help for specific command
        for (uint32_t i = 0; i < command_count; i++) {
            if (strcmp(command_table[i]->name, argv[1]) == 0) {
                terminal_writestring(command_table[i]->name);
                terminal_writestring(" - ");
                terminal_writestring(command_table[i]->description);
                terminal_writestring("\n");
                return 0;
            }
        }
        
        terminal_writestring("No help available for '");
        terminal_writestring(argv[1]);
        terminal_writestring("'\n");
        return -1;
    }
    
    // Display all commands
    terminal_writestring("Available commands:\n");
    for (uint32_t i = 0; i < command_count; i++) {
        terminal_writestring("  ");
        terminal_writestring(command_table[i]->name);
        terminal_writestring(" - ");
        terminal_writestring(command_table[i]->description);
        terminal_writestring("\n");
    }
    
    return 0;
}

/* Built-in command: echo */
int shell_cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        terminal_writestring(argv[i]);
        if (i < argc - 1) {
            terminal_writestring(" ");
        }
    }
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: exit */
int shell_cmd_exit(int argc, char** argv) {
    terminal_writestring("Exiting shell...\n");
    
    // In a real implementation, this would exit the shell
    // For now, we'll just return
    
    return 0;
}

/* Built-in command: clear */
int shell_cmd_clear(int argc, char** argv) {
    terminal_clear();
    
    return 0;
}

/* Built-in command: ls */
int shell_cmd_ls(int argc, char** argv) {
    char* path = (argc > 1) ? argv[1] : current_dir;
    
    // In a real implementation, this would list directory contents
    // For now, we'll just print a message
    terminal_writestring("Listing directory: ");
    terminal_writestring(path);
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: cd */
int shell_cmd_cd(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: cd <directory>\n");
        return -1;
    }
    
    // In a real implementation, this would change the current directory
    // For now, we'll just update the current_dir variable
    strcpy(current_dir, argv[1]);
    
    return 0;
}

/* Built-in command: cat */
int shell_cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: cat <file>\n");
        return -1;
    }
    
    // In a real implementation, this would display file contents
    // For now, we'll just print a message
    terminal_writestring("Displaying file: ");
    terminal_writestring(argv[1]);
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: mkdir */
int shell_cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: mkdir <directory>\n");
        return -1;
    }
    
    // In a real implementation, this would create a directory
    // For now, we'll just print a message
    terminal_writestring("Creating directory: ");
    terminal_writestring(argv[1]);
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: rm */
int shell_cmd_rm(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: rm <file>\n");
        return -1;
    }
    
    // In a real implementation, this would remove a file
    // For now, we'll just print a message
    terminal_writestring("Removing file: ");
    terminal_writestring(argv[1]);
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: rmdir */
int shell_cmd_rmdir(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: rmdir <directory>\n");
        return -1;
    }
    
    // In a real implementation, this would remove a directory
    // For now, we'll just print a message
    terminal_writestring("Removing directory: ");
    terminal_writestring(argv[1]);
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: touch */
int shell_cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: touch <file>\n");
        return -1;
    }
    
    // In a real implementation, this would create an empty file
    // For now, we'll just print a message
    terminal_writestring("Creating file: ");
    terminal_writestring(argv[1]);
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: ps */
int shell_cmd_ps(int argc, char** argv) {
    // In a real implementation, this would display process status
    // For now, we'll just print a message
    terminal_writestring("Process status:\n");
    terminal_writestring("  PID  TTY  TIME     CMD\n");
    terminal_writestring("    1  tty1  00:00:01 init\n");
    terminal_writestring("    2  tty1  00:00:00 shell\n");
    
    return 0;
}

/* Built-in command: kill */
int shell_cmd_kill(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: kill <pid>\n");
        return -1;
    }
    
    // In a real implementation, this would terminate a process
    // For now, we'll just print a message
    terminal_writestring("Terminating process: ");
    terminal_writestring(argv[1]);
    terminal_writestring("\n");
    
    return 0;
}

/* Built-in command: ifconfig */
int shell_cmd_ifconfig(int argc, char** argv) {
    // In a real implementation, this would display network interface configuration
    // For now, we'll just print a message
    terminal_writestring("Network interfaces:\n");
    terminal_writestring("  lo: 127.0.0.1 netmask 255.0.0.0\n");
    terminal_writestring("  eth0: 192.168.1.100 netmask 255.255.255.0\n");
    
    return 0;
}

/* Built-in command: ping */
int shell_cmd_ping(int argc, char** argv) {
    if (argc < 2) {
        terminal_writestring("Usage: ping <host>\n");
        return -1;
    }
    
    // In a real implementation, this would send ICMP echo requests
    // For now, we'll just print a message
    terminal_writestring("Pinging ");
    terminal_writestring(argv[1]);
    terminal_writestring("...\n");
    
    return 0;
}

/* Built-in command: netstat */
int shell_cmd_netstat(int argc, char** argv) {
    // In a real implementation, this would display network connections
    // For now, we'll just print a message
    terminal_writestring("Active Internet connections:\n");
    terminal_writestring("  Proto  Local Address  Foreign Address  State\n");
    terminal_writestring("  tcp    0.0.0.0:22     0.0.0.0:*        LISTEN\n");
    
    return 0;
}
