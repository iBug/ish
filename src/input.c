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

char* get_input(void) {
    static char cmd[MAX_CMD_LEN];
    static char prompt[MAX_PROMPT_LEN];
    char *s;

    // Prepare prompt
    s = getcwd(cwd, MAX_PATH);
    snprintf(prompt, sizeof(prompt), "%s $ ", cwd);

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
