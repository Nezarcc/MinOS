#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <stdint.h>

/* Maximum script size */
#define SCRIPT_MAX_SIZE 16384

/* Script variable structure */
typedef struct {
    char name[32];
    char value[256];
} script_var_t;

/* Initialize the scripting engine */
void script_init(void);

/* Execute a script file */
int script_execute_file(const char* filename);

/* Execute a script string */
int script_execute_string(const char* script);

/* Set a script variable */
int script_set_var(const char* name, const char* value);

/* Get a script variable */
const char* script_get_var(const char* name);

/* Parse and evaluate an expression */
int script_eval_expr(const char* expr, char* result, int result_size);

#endif /* SCRIPTING_H */
