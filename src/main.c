// File: main.c
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

#define MAX_LEN 8192
#define MAX_ARGS 32
#define MAX_ARG_LEN 256

#ifdef D
#define DEBUG(a...) fprintf(stderr, "DEBUG: " a)
#else
#define DEBUG(a...)
#endif

char prompt[MAX_LEN];
char argv[MAX_ARGS][MAX_ARG_LEN];

char *argv[MAX_ARGS + 1];
int cmdlen, argc, argcount;

int main(int _argc, char * const * _argv) {
    char *s;
    int i, j;
    int is_pipe, redir_mode, rredir = 0, wredir = 0, pipefd[2];

    while (1) {
        char *cmd = get_input();
        if (!cmd) {
            if (isatty(STDIN_FILENO) && isatty(STDERR_FILENO))
                fprintf(stderr, "exit\n");
            exit(0);
        }
        cmdlen = strlen(cmd);

        // Split arguments
        i = 0;
        // Loop first in case of ';' to handle
        while (i < cmdlen) {
            is_pipe = 0, redir_mode = 0, argc = 0, argcount = 0;
            while (i < cmdlen) {
                // Skip all control stuff
                while (cmd[i] <= ' ' || cmd[i] == '\x7F') {
                    cmd[i] = 0;
                    i++;
                }

                // Make a pointer to all following non-control characters
                int prev_redir_mode = redir_mode, x = 0;
                char context = 0, // Handle quotes
                     *parg = args[argcount];
                s = cmd + i;
                while (i < cmd) {
                // while (cmd[i] > ' ' && cmd[i] < '\x7F') {
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
                    } else if (cmd[i] == '>') {
                        if (cmd[i + 1] == '>') {
                            redir_mode = 'a';
                            cmd[i] = cmd[i + 1] = 0;
                            i += 2;
                        } else {
                            redir_mode = 'w';
                            cmd[i] = 0;
                            i++;
                        }
                        break;
                    }
                    i++;
                }
                if (*s) { // Prevent empty stuff
                    if (prev_redir_mode == 'r') {
                        if (rredir > 0)
                            close(rredir); // Avoid jamming
                        cmd[i] = 0;
                        FILE *fp = fopen(s, "r");
                        DEBUG("%x: %s\n", fp, s);
                        rredir = fileno(fp);
                        continue;
                    } else if (prev_redir_mode == 'w') {
                        if (wredir > 0)
                            close(wredir);
                        cmd[i] = 0;
                        FILE *fp = fopen(s, "w");
                        wredir = fileno(fp);
                        continue;
                    } else if (prev_redir_mode == 'a') {
                        if (wredir > 0)
                            close(wredir);
                        cmd[i] = 0;
                        FILE *fp = fopen(s, "a");
                        wredir = fileno(fp);
                        continue;
                    } else {
                        argv[argcount++] = s;
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
                if (pipe(pipefd) == -1)
                    fprintf(stderr, "pipe: %s\n", strerror(errno));
                DEBUG("pipe: %d -> %d\n", pipefd[1], pipefd[0]);
                wredir = pipefd[1];
            }

            // Execute the command
            // If the command does not end with a pipe, use fork(2)
            pid_t fork_pid = fork();

            if (fork_pid) {
                // Parent
                // Clear the pipe record
                if (rredir > 0) {
                    close(rredir); // Shut down the old pipe
                }
                if (is_pipe) {
                    rredir = pipefd[0]; // Record the new pipe for next loop
                }
                if (wredir > 0) {
                    close(wredir);
                    wredir = 0;
                }

                // If the last command has a pipe, don't wait
                if (!is_pipe) {
                    // Wait for the child to complete
                    int status;
                    waitpid(fork_pid, &status, 0);
                    DEBUG("Child exit code: %d\n", WEXITSTATUS(status));
                    if (WEXITSTATUS(status) != 0) {
                        fprintf(stderr, "%d exited: %d\n", fork_pid, WEXITSTATUS(status));
                    }
                }
            }
            else {
                // Child - Go execve
                if (rredir > 0) {
                    // The last command has an open pipe, connect it with stdin:
                    dup2(rredir, 0);
                    close(rredir);
                }
                if (wredir > 0) {
                    // The current command has an outgoing redirection
                    dup2(wredir, 1);
                    close(wredir);
                }
                execvp(argv[0], argv);

                // Normally unreachable - something's wrong
                fprintf(stderr, "%s: %s\n", argv[0], strerror(errno));
                exit(127);
            }
        }
    }
    return 0;
}
