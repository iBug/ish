// File: parsing.c
// Author: iBug

#include "parsing.h"

#include <string.h>
#include "variables.h"

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
        case '#': *out = '#'; return 1;
        case ' ': *out = ' '; return 1;
        case 'e': *out = '\x1B'; return 1;
        case 'n': *out = '\n'; return 1;
        case 'r': *out = '\r'; return 1;
        case 't': *out = '\t'; return 1;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': {
            // Parse up to 3 octal digits
            int ch = *s - '0';
            s++;
            if (*s >= '0' && *s <= '7') {
                ch = 8 * ch + *s - '0';
            } else {
                *out = ch;
                return 1;
            }
            s++;
            if (*s >= '0' && *s <= '7') {
                ch = 8 * ch + *s - '0';
            } else {
                *out = ch;
                return 2;
            }
            *out = ch;
            return 3;
        } break;
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

int expand_token(char* out, const char* s, size_t maxlen) {
    if (*s == '\\') {
        return escape_char(out, s + 1) + 1;
    } else if (*s == '$') {
        char varname[MAX_VAR_NAME] = {};
        const char *varvalue, *s_orig = s;
        int j = 0, brace = 0;
        if (s[1] == '{') {
            brace = 1;
            s += 2;
        } else {
            s++;
        }
        for (;j < MAX_VAR_NAME; s++, j++) {
            if ((*s >= 'A' && *s <= 'Z') ||
                (*s >= 'a' && *s <= 'z') ||
                (*s >= '0' && *s <= '9') ||
                *s == '_') {
                    varname[j] = *s;
            } else if (brace) {
                if (*s == '}') {
                    s++;
                    break;
                }
            } else break;
        }
        varvalue = get_variable(varname);
        if (varvalue) {
            strncpy(out, varvalue, maxlen - 1);
        }
        return s - s_orig;
    } else { // Nothing to expand
        *out = *s;
        return 1;
    }
}
