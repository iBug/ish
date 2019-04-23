// File: input.c
// Author: iBug

#include "input.h"
#include "global.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

static char username[64],
            hostname[64];

char* get_input(char *buf, int mode) {
    static char cmd[MAX_CMD_LEN];
    static char prompt[MAX_PROMPT_LEN];
    if (!buf)
        buf = cmd;
    char *s;

    // Prepare prompt
    s = getcwd(cwd, MAX_PATH);
    if (!username[0]) {
        getlogin_r(username, sizeof(username));
        if (!username[0]) {
            const char *pusername = getenv("USER");
            if (pusername)
                strncpy(username, pusername, sizeof(username) - 1);
            else
                strcpy(username, "<unknown>");
        }
    }
    if (!hostname[0]) {
        gethostname(hostname, sizeof(hostname));
        hostname[sizeof(hostname) - 1] = 0;
    }

    // mode = prompt type ${PS$mode}
    if (mode == 1) { // Regular, $PS1
#ifdef COLOR_PROMPT
        snprintf(prompt, sizeof(prompt), "\1\x1B[32;1m\2%s@%s\1\x1B[0m\2:\1\x1B[34;1m\2%s\1\x1B[0m\2 $ ", username, hostname, cwd);
#else
        snprintf(prompt, sizeof(prompt), "%s $ ", cwd);
#endif
    } else {
        strcpy(prompt, "> "); // Continuation, $PS2
    }

    if (isatty(STDIN_FILENO)) {
#ifdef HAVE_READLINE
        s = readline(prompt);
        if (!s) {
            return NULL;
        }
        int sl = strlen(s);
        if (sl >= MAX_CMD_LEN) {
            s[MAX_CMD_LEN - 1] = 0;
        }
        strcpy(buf, s);
        add_history(buf);
        free(s);
#else
        fprintf(stderr, "%s", prompt);
        fflush(stderr);

        // Get input
        s = fgets(buf, MAX_CMD_LEN, stdin);
        if (!s) {
            return NULL;
        }
        int cmdlen = strlen(buf);
        if (buf[cmdlen - 1] == '\n') {
            buf[--cmdlen] = 0;
        } else {
            // Truncate the rest of the line
            int ch;
            do {
                ch = getchar();
            } while (ch >= 0 && ch != '\n');
        }
#endif
    } else {
        s = fgets(buf, MAX_CMD_LEN, stdin);
        if (!s) {
            return NULL;
        }
        int cmdlen = strlen(buf);
        if (buf[cmdlen - 1] == '\n') {
            buf[--cmdlen] = 0;
        } else {
            // Truncate the rest of the line
            int ch;
            do {
                ch = getchar();
            } while (ch >= 0 && ch != '\n');
        }
    }

    // Return the buffer
    return buf;
}
