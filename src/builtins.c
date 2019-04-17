// File: builtins.c
// Author: iBug

#include "builtins.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int process_builtin(int argc, char * const * args) {
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
    else if (!strcmp(cmd, "exec")) {
        if (argc < 2) {
            fprintf(stderr, "exec: No command");
            return 1;
        }
        execvp(args[1], (char * const *)args + 1);
        fprintf(stderr, "%s: %s\n", args[1], strerror(errno));
    }
    else {
        return 0; // Not a built-in
    }
    return 1; // True - this is a built-in
}
