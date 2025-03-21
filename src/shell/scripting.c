#include "scripting.h"
#include "../kernel/kernel.h"
#include "../fs/file.h"
#include <stdint.h>
#include <string.h>

/* Script variables */
static script_var_t variables[64];
static uint32_t variable_count = 0;

/* Initialize the scripting engine */
void script_init() {
    terminal_writestring("Initializing scripting engine...\n");
    
    // Clear variables
    for (int i = 0; i < 64; i++) {
        variables[i].name[0] = '\0';
        variables[i].value[0] = '\0';
    }
    
    variable_count = 0;
    
    // Set some default variables
    script_set_var("PATH", "/bin:/usr/bin");
    script_set_var("HOME", "/home/user");
    script_set_var("USER", "user");
    script_set_var("SHELL", "/bin/minsh");
    
    terminal_writestring("Scripting engine initialized\n");
}

/* Execute a script file */
int script_execute_file(const char* filename) {
    // In a real implementation, this would read and execute a script file
    // For now, we'll just print a message
    terminal_writestring("Executing script file: ");
    terminal_writestring(filename);
    terminal_writestring("\n");
    
    return 0;
}

/* Execute a script string */
int script_execute_string(const char* script) {
    if (!script) {
        return -1;
    }
    
    // Make a copy of the script
    char script_copy[SCRIPT_MAX_SIZE];
    strncpy(script_copy, script, SCRIPT_MAX_SIZE - 1);
    script_copy[SCRIPT_MAX_SIZE - 1] = '\0';
    
    // Parse and execute each line
    char* line = strtok(script_copy, "\n");
    while (line) {
        // Skip empty lines and comments
        if (line[0] != '\0' && line[0] != '#') {
            // Process line
            // In a real implementation, this would parse and execute the line
            // For now, we'll just print it
            terminal_writestring("Executing: ");
            terminal_writestring(line);
            terminal_writestring("\n");
        }
        
        // Get next line
        line = strtok(NULL, "\n");
    }
    
    return 0;
}

/* Set a script variable */
int script_set_var(const char* name, const char* value) {
    if (!name || !value) {
        return -1;
    }
    
    // Check if variable already exists
    for (uint32_t i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            // Update existing variable
            strncpy(variables[i].value, value, 255);
            variables[i].value[255] = '\0';
            return 0;
        }
    }
    
    // Add new variable
    if (variable_count >= 64) {
        return -1; // Too many variables
    }
    
    strncpy(variables[variable_count].name, name, 31);
    variables[variable_count].name[31] = '\0';
    
    strncpy(variables[variable_count].value, value, 255);
    variables[variable_count].value[255] = '\0';
    
    variable_count++;
    
    return 0;
}

/* Get a script variable */
const char* script_get_var(const char* name) {
    if (!name) {
        return NULL;
    }
    
    // Find variable
    for (uint32_t i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    
    return NULL; // Variable not found
}

/* Parse and evaluate an expression */
int script_eval_expr(const char* expr, char* result, int result_size) {
    if (!expr || !result || result_size <= 0) {
        return -1;
    }
    
    // In a real implementation, this would parse and evaluate the expression
    // For now, we'll just copy the expression to the result
    strncpy(result, expr, result_size - 1);
    result[result_size - 1] = '\0';
    
    return 0;
}
