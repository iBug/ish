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
            for (argcount = 0; i < cmdlen; argcount++) {
                // Skip all control stuff
                while (cmd[i] <= ' ' || cmd[i] == '\x7F') {
                    cmd[i] = 0;
                    i++;
                }

                // Make a pointer to all following non-control characters
                s = cmd + i;
                argv[argcount] = s;
                while (cmd[i] > ' ' && cmd[i] < '\x7F') {
                    if (cmd[i] == ';') {
                        cmd[i] = 0;
                        break;
                    }
                    i++;
                }

                // Get ready to execute!
                if (cmd[i] == 0) {
                    argcount++; // The last pointer ...
                    break;
                }
            }
            if (argcount == 0) {
                printf("No command\n");
                continue;
            }
            argv[argcount] = NULL; // ... must be NULL
            DEBUG("argcount = %d\n", argcount);
            DEBUG("$0 = %s\n", argv[0]);

            // Check for builtin commands
            if (process_builtin(argcount, argv))
                continue;

            // Execute the command
            pid_t fork_pid = fork();

            if (fork_pid) {
                // Parent
                int status, ecode;
                waitpid(fork_pid, &status, 0);
                ecode = WEXITSTATUS(status);
                DEBUG("Child exit code: %d\n", ecode);
                if (ecode == 233) {
                    printf("OSLab2: %s: not found\n", argv[0]);
                }
            }
            else {
                // Child - Go execve
                int err = execvp(argv[0], argv);

                // Normally unreachable - something's wrong
                DEBUG("exec(3): %d\n", err);
                exit(233);
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
