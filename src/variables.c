// File: variables.c
// Author: iBug

#include "variables.h"
#include <stdlib.h>
#include <string.h>

variable_t variables[MAX_VARS];
unsigned var_count = 0;

const char* get_variable(const char* name) {
    for (int i = 0; i < var_count; i++)
        if (!strcmp(name, variables[i].name))
            return variables[i].value;
    return getenv(name);
}

const char* set_variable(const char* name, const char* value, int env) {
    if (!value) // Empty pointer?
        return unset_variable(name);

    // If setting environment variable
    if (env) {
        if (setenv(name, value, 1))
            return NULL;
        return value;
    }

    // If variable exists
    for (int i = 0; i < var_count; i++)
        if (!strcmp(name, variables[i].name)) {
            strncpy(variables[i].value, value, MAX_VAR_VALUE);
            variables[i].value[MAX_VAR_VALUE - 1] = 0;
            return variables[i].value;
        }

    if (var_count == MAX_VARS) // Registry full
        return NULL;

    // Insert new variable
    int n = var_count++;
    strncpy(variables[n].name, name, MAX_VAR_NAME);
    strncpy(variables[n].value, value, MAX_VAR_VALUE);
    variables[n].name[MAX_VAR_NAME - 1] = 0;
    variables[n].value[MAX_VAR_VALUE - 1] = 0;
    return variables[n].value;
}

const char* unset_variable(const char* name) {
    unsetenv(name);
    for (int i = 0; i < var_count; i++)
        if (!strcmp(name, variables[i].name)) {
            int n = --var_count;
            if (i == n) return NULL; // Removing the last one
            strcpy(variables[i].name, variables[n].name);
            strcpy(variables[i].value, variables[n].value);
            break;
        }
    return NULL;
}
