// File: shell.c
// Author: iBug

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "input.h"
#include "builtins.h"

#define MAX_LEN 256
#define MAX_ARGS 8
#define MAX_ARG_LEN 64

#ifdef D
#define DEBUG(a...) fprintf(stderr, "DEBUG: " a)
#else
#define DEBUG(a...)
#endif

char cwd[MAX_LEN];
char cmd[MAX_LEN];
char prompt[MAX_LEN];
char arg[MAX_ARGS][MAX_ARG_LEN];

char *argv[MAX_ARGS + 1];
int cmdlen, argcount;

int main(int _argc, char** _argv, char** _envp) {
    char *s;
    int i, j;
    int is_pipe, redir_mode, rredir = 0, wredir, pipefd[2];

    while (1) {
        char *cmd = get_input();
        if (!cmd) {
            fprintf(stderr, "exit\n");
            exit(0);
        }
        cmdlen = strlen(cmd);

        // Split arguments
        i = 0;
        // Loop first in case of ';' to handle
        while (i < cmdlen) {
            is_pipe = 0, redir_mode = 0;
            for (argcount = 0; i < cmdlen;) {
                // Skip all control stuff
                while (cmd[i] <= ' ' || cmd[i] == '\x7F') {
                    cmd[i] = 0;
                    i++;
                }

                // Make a pointer to all following non-control characters
                int prev_redir_mode = redir_mode;
                redir_mode = 0;
                s = cmd + i;
                while (cmd[i] > ' ' && cmd[i] < '\x7F') {
                    if (cmd[i] == ';') {
                        cmd[i] = 0;
                        break;
                    } else if (cmd[i] == '|') {
                        cmd[i] = 0;
                        is_pipe = 1;
                        break;
                    } else if (cmd[i] == '<') {
                        redir_mode = 'r';
                        cmd[i] = 0;
                        i++; // Continue splitting arguments
                        break;
                    }
                    i++;
                }
                if (*s) {
                    if (prev_redir_mode == 'r') {
                        FILE *fp = fopen(s, "r");
                        rredir = fileno(fp);
                    } else {
                        argv[argcount++] = s; // Prevent an empty argument
                    }
                }

                // Get ready to execute!
                if (cmd[i] == 0) {
                    break;
                }
            }
            if (argcount == 0) {
                printf("No command\n");
                continue;
            }
            argv[argcount] = NULL; // The last pointer of ARGV must be NULL
            DEBUG("argcount = %d\n", argcount);
            for (j = 0; j < argcount; j++)
                DEBUG("$%d = %s\n", j, argv[j]);

            // Check for builtin commands
            if (process_builtin(argcount, argv))
                continue;

            // Handle pipes
            pipefd[0] = pipefd[1] = 0;
            if (is_pipe) {
                int err = pipe(pipefd); // Get a pipe
                DEBUG("pipe: %d -> %d\n", pipefd[1], pipefd[0]);
            }
            wredir = pipefd[1];

            // Execute the command
            // If the command does not end with a pipe, use fork(2)
            pid_t fork_pid = fork();

            if (fork_pid) {
                // Parent
                // Clear the pipe record
                if (is_pipe) {
                    close(wredir);
                    wredir = 0;
                    if (rredir > 0) {
                        close(rredir); // Shut down the old pipe
                    }
                    rredir = pipefd[0]; // Record the new pipe for next loop
                }

                // If the last command has a pipe, don't wait
                if (!is_pipe) {
                    // Wait for the child to complete
                    int status;
                    waitpid(fork_pid, &status, 0);
                    DEBUG("Child exit code: %d\n", WEXITSTATUS(ecode));
                }
            }
            else {
                // Child - Go execve
                if (rredir > 0) {
                    // The last command has an open pipe, connect it with stdin:
                    dup2(rredir, 0);
                    close(rredir);
                }
                if (is_pipe) {
                    // The current command has an outgoing pipe
                    dup2(wredir, 1);
                    close(wredir);
                }
                int err = execvp(argv[0], argv);

                // Normally unreachable - something's wrong
                fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
                exit(127);
            }
        }
    }
    return 0;
}
