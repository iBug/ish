// File: variables.h
// Author: iBug

#ifndef _VARIABLES_H
#define _VARIABLES_H

#define MAX_VARS 64
#define MAX_VAR_NAME 64
#define MAX_VAR_VALUE 256

typedef struct variable {
    char name[MAX_VAR_NAME];
    char value[MAX_VAR_VALUE];
} variable_t;

extern variable_t variables[MAX_VARS];

const char* get_variable(const char* name);
const char* set_variable(const char* name, const char* value, int env);

#endif
