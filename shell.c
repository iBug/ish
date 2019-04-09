// File: ish.c
// Author: iBug

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define MAX_LEN 256
#define MAX_ARGS 8
#define MAX_ARG_LEN 64

#ifdef D
#define DEBUG(a...) fprintf(stderr, "OSLab2 DEBUG: " a)
#else
#define DEBUG(a...)
#endif

char cwd[MAX_LEN];
char cmd[MAX_LEN];
char prompt[MAX_LEN];
char arg[MAX_ARGS][MAX_ARG_LEN];

char *argv[MAX_ARGS + 1];
int cmdlen, argcount;

int process_builtin(int, char const * const * args);

int main(int _argc, char** _argv, char** _envp) {
    char *s;
    int i, j;
    int is_pipe, read_pipe = 0, write_pipe, pipefd[2];

    while (1) {
        // Prepare prompt
        fprintf(stderr, "OSLab2-> ");
        fflush(stderr);

        // Get input
        s = fgets(cmd, MAX_LEN, stdin);
        if (!s) {
            puts("exit");
            return 0;
        }
        cmdlen = strlen(cmd);
        if (cmd[cmdlen - 1] == '\n') {
            cmd[--cmdlen] = 0;
        }

        // Split arguments
        i = 0;
        // Loop first in case of ';' to handle
        while (i < cmdlen) {
            for (is_pipe = 0, argcount = 0; i < cmdlen;) {
                // Skip all control stuff
                while (cmd[i] <= ' ' || cmd[i] == '\x7F') {
                    cmd[i] = 0;
                    i++;
                }

                // Make a pointer to all following non-control characters
                s = cmd + i;
                while (cmd[i] > ' ' && cmd[i] < '\x7F') {
                    if (cmd[i] == ';') {
                        cmd[i] = 0;
                        break;
                    } else if (cmd[i] == '|') {
                        cmd[i] = 0;
                        is_pipe = 1;
                        break;
                    }
                    i++;
                }
                if (*s)
                    argv[argcount++] = s; // Prevent an empty argument

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
            if (process_builtin(argcount, (const char * const *)argv))
                continue;

            // Handle pipes
            if (is_pipe) {
                int err = pipe(pipefd); // Get a pipe
                write_pipe = pipefd[1];
                DEBUG("pipe: %d -> %d\n", pipefd[1], pipefd[0]);
            }

            // Execute the command
            // If the command does not end with a pipe, use fork(2)
            pid_t fork_pid = fork();

            if (fork_pid) {
                // Parent
                // Clear the pipe record
                if (is_pipe) {
                    close(write_pipe);
                    write_pipe = 0;
                    if (read_pipe > 0) {
                        close(read_pipe);
                    }
                    read_pipe = pipefd[0];
                }

                // If the last command has a pipe, don't wait
                if (!is_pipe) {
                    // Wait for the child to complete
                    int status, ecode;
                    waitpid(fork_pid, &status, 0);
                    ecode = WEXITSTATUS(status);
                    DEBUG("Child exit code: %d\n", ecode);
                }
            }
            else {
                // Child - Go execve
                if (read_pipe > 0) {
                    // The last command has an open pipe, connect it with stdin:
                    dup2(read_pipe, 0);
                }
                if (is_pipe) {
                    // The current command has an outgoing pipe
                    dup2(write_pipe, 1);
                    close(read_pipe);
                }
                int err = execvp(argv[0], argv);

                // Normally unreachable - something's wrong
                fprintf(stderr, "OSLab2: %s: %s\n", argv[0], strerror(errno));
                exit(127);
            }
        }
    }
    return 0;
}

int process_builtin(int argc, char const * const * args) {
    const char *cmd = args[0];
    if (!strlen(cmd)) {
        return 0; // wat?
    }
    else if (!strcmp(cmd, "cd")) {
        const char *target;
        if (argc < 2) {
            target = getenv("HOME");
        } else {
            target = args[1];
        }
        int result = chdir(target);
        if (result) {
            fprintf(stderr, "cd: %s\n", strerror(errno));
        }
    }
    else if (!strcmp(cmd, "exit")) {
        exit(0);
    }
    else {
        return 0; // Not a built-in
    }
    return 1; // True - this is a built-in
}
