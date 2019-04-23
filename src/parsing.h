// File: parsing.h
// Author: iBug

#ifndef _PARSING_H
#define _PARSING_H
#include <stddef.h>

int escape_char(char* out, const char* s);
int expand_token(char* out, const char* in, size_t);

#endif
