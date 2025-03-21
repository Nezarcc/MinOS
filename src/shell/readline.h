#ifndef READLINE_H
#define READLINE_H

#include <stdint.h>

/* Initialize the readline library */
void readline_init(void);

/* Read a line from the terminal with editing capabilities */
char* readline(const char* prompt);

/* Add a line to the history */
void add_history(const char* line);

/* Get a line from history by index */
char* get_history_line(int index);

/* Get the number of lines in history */
int get_history_length(void);

/* Clear the history */
void clear_history(void);

/* Set completion function */
void set_completion_function(char** (*completion_func)(const char* text, int start, int end));

#endif /* READLINE_H */
