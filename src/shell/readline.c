#include "readline.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <string.h>

/* Maximum line length */
#define MAX_LINE_LENGTH 256

/* Maximum history size */
#define MAX_HISTORY_SIZE 100

/* History buffer */
static char history[MAX_HISTORY_SIZE][MAX_LINE_LENGTH];
static int history_length = 0;
static int history_index = 0;

/* Completion function */
static char** (*completion_callback)(const char* text, int start, int end) = NULL;

/* Initialize the readline library */
void readline_init() {
    terminal_writestring("Initializing readline library...\n");
    
    // Clear history
    for (int i = 0; i < MAX_HISTORY_SIZE; i++) {
        history[i][0] = '\0';
    }
    
    history_length = 0;
    history_index = 0;
    
    terminal_writestring("Readline library initialized\n");
}

/* Read a line from the terminal with editing capabilities */
char* readline(const char* prompt) {
    static char line[MAX_LINE_LENGTH];
    int pos = 0;
    
    // Display prompt
    terminal_writestring(prompt);
    
    // Clear line buffer
    memset(line, 0, MAX_LINE_LENGTH);
    
    // Read characters until newline or buffer full
    while (pos < MAX_LINE_LENGTH - 1) {
        // In a real implementation, this would read from the keyboard
        // and handle special keys (arrows, backspace, etc.)
        // For now, we'll just simulate a simple input
        
        // Simulated input for testing
        if (pos == 0) {
            strcpy(line, "help");
            pos = 4;
            break;
        }
        
        // Add character to line
        line[pos++] = 'a'; // Placeholder
        
        // Echo character
        terminal_putchar('a');
        
        // Break on newline
        if (line[pos-1] == '\n') {
            line[pos-1] = '\0'; // Remove newline
            pos--;
            break;
        }
    }
    
    // Null-terminate line
    line[pos] = '\0';
    
    // Add newline
    terminal_writestring("\n");
    
    return line;
}

/* Add a line to the history */
void add_history(const char* line) {
    // Don't add empty lines
    if (!line || line[0] == '\0') {
        return;
    }
    
    // Don't add duplicate of last line
    if (history_length > 0 && strcmp(history[(history_index - 1) % MAX_HISTORY_SIZE], line) == 0) {
        return;
    }
    
    // Add line to history
    strncpy(history[history_index], line, MAX_LINE_LENGTH - 1);
    history[history_index][MAX_LINE_LENGTH - 1] = '\0';
    
    // Update indices
    history_index = (history_index + 1) % MAX_HISTORY_SIZE;
    if (history_length < MAX_HISTORY_SIZE) {
        history_length++;
    }
}

/* Get a line from history by index */
char* get_history_line(int index) {
    if (index < 0 || index >= history_length) {
        return NULL;
    }
    
    // Calculate actual index in circular buffer
    int actual_index = (history_index - history_length + index) % MAX_HISTORY_SIZE;
    if (actual_index < 0) {
        actual_index += MAX_HISTORY_SIZE;
    }
    
    return history[actual_index];
}

/* Get the number of lines in history */
int get_history_length() {
    return history_length;
}

/* Clear the history */
void clear_history() {
    for (int i = 0; i < MAX_HISTORY_SIZE; i++) {
        history[i][0] = '\0';
    }
    
    history_length = 0;
    history_index = 0;
}

/* Set completion function */
void set_completion_function(char** (*completion_func)(const char* text, int start, int end)) {
    completion_callback = completion_func;
}

/* Default completion function */
char** default_completion(const char* text, int start, int end) {
    // In a real implementation, this would provide tab completion
    // For now, we'll just return NULL
    return NULL;
}
