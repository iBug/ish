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

char* get_input(void) {
    static char cmd[MAX_CMD_LEN];
    static char prompt[MAX_PROMPT_LEN];
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
#ifdef COLOR_PROMPT
    snprintf(prompt, sizeof(prompt), "\x1B[32;1m%s@%s\x1B[0m:\x1B[34;1m%s\x1B[0m $ ", username, hostname, cwd);
#else
    snprintf(prompt, sizeof(prompt), "%s $ ", cwd);
#endif

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
        strcpy(cmd, s);
        add_history(cmd);
        free(s);
#else
        fprintf(stderr, "%s", prompt);
        fflush(stderr);

        // Get input
        s = fgets(cmd, MAX_CMD_LEN, stdin);
        if (!s) {
            return NULL;
        }
        int cmdlen = strlen(cmd);
        if (cmd[cmdlen - 1] == '\n') {
            cmd[--cmdlen] = 0;
        } else {
            // Truncate the rest of the line
            int ch;
            do {
                ch = getchar();
            } while (ch >= 0 && ch != '\n');
        }
#endif
    } else {
        s = fgets(cmd, MAX_CMD_LEN, stdin);
        if (!s) {
            return NULL;
        }
        int cmdlen = strlen(cmd);
        if (cmd[cmdlen - 1] == '\n') {
            cmd[--cmdlen] = 0;
        } else {
            // Truncate the rest of the line
            int ch;
            do {
                ch = getchar();
            } while (ch >= 0 && ch != '\n');
        }
    }

    return cmd;
}
