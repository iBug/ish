// File: parsing.c
// Author: iBug

#include "parsing.h"

int escape_char(char* out, const char* s) {
    if (!s || !*s) return 0;
    switch(s[0]) {
        case '\\': *out = '\\'; return 1;
        case '\"': *out = '\"'; return 1;
        case '\'': *out = '\''; return 1;
        case '|': *out = '|'; return 1;
        case '>': *out = '>'; return 1;
        case '<': *out = '<'; return 1;
        case '&': *out = '&'; return 1;
        case '$': *out = '$'; return 1;
        case ' ': *out = ' '; return 1;
        case '0': *out = '\0'; return 1;
        case 'e': *out = '\x1B'; return 1;
        case 'n': *out = '\n'; return 1;
        case 'r': *out = '\r'; return 1;
        case 't': *out = '\t'; return 1;

        case 'x': case 'X': { // Parse up to 2 hex digits
            s++;
            int ch = 0;
            if (*s >= '0' && *s <= '9') {
                ch = *s - '0';
            } else if (*s >= 'A' && *s <= 'F') {
                ch = 10 + *s - 'A';
            } else if (*s >= 'a' && *s <= 'f') {
                ch = 10 + *s - 'a';
            } else {
                *out = 0;
                return 1;
            }
            s++;
            if (*s >= '0' && *s <= '9') {
                ch = 16 * ch + *s - '0';
            } else if (*s >= 'A' && *s <= 'F') {
                ch = 16 * ch + 10 + *s - 'A';
            } else if (*s >= 'a' && *s <= 'f') {
                ch = 16 * ch + 10 + *s - 'a';
            } else {
                *out = ch;
                return 2;
            }
            *out = ch;
            return 3;
        }
        default: *out = '\\'; return 0; // Unrecognized escape sequence - do nothing
    }
}
